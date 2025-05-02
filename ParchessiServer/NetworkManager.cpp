#include "NetworkManager.h"
#include "SQLManager.h"

sf::Packet& operator>>(sf::Packet& _packet, packetType& _type)
{
    int temp;
    _packet >> temp;
    _type = static_cast<packetType>(temp);

    return _packet;
};
sf::Packet& operator<<(sf::Packet& _packet, const sf::TcpSocket& _socket)
{
    std::string remoteAddress = _socket.getRemoteAddress().value().toString();

    _packet << remoteAddress
            << _socket.getRemotePort();

    return _packet;
}
sf::IpAddress StringToIpAddress(const std::string& ipAdress)
{
    unsigned short a, b, c, d;

    if (sscanf_s(ipAdress.c_str(), R"(%u.%u.%u.%u)", &a, &b, &c, &d) != 4)
        return sf::IpAddress::Any;

    return sf::IpAddress(a, b, c, d);
}
// The Unsigned Short is the Port of the Address
sf::Packet& operator>>(sf::Packet& _packet, std::pair<sf::IpAddress, unsigned short>& _address)
{
    std::string ipAdress;

    if (!(_packet >> ipAdress >> _address.second))
    {
        std::cerr << "Error on reading IP Adress / Port" << std::endl;
        return _packet;
    }

    _address.first = StringToIpAddress(ipAdress);

    return _packet;
}


Room* NetworkManager::GetRoomByCode(std::string roomCode)
{
    for (auto room : rooms)
    {
        if (room->GetRoomCode() == roomCode)
        {
            return room;
        }
    }
    return nullptr;
}

std::string NetworkManager::GetRoomCodeOfClient(Client* client)
{
    for (auto room : rooms)
    {
        if (room->HasClient(client)) 
        {
            return room->GetRoomCode();
        }
    }
    return "";
}

void NetworkManager::OnReceiveLogin(sf::Packet packet, Client* client)
{
    std::string username, password;
    packet >> username >> password;

    int num = SQL_MANAGER.CheckLogin(username, password);
    SendAuthenticationResult(client, num);
}

void NetworkManager::OnReceiveRegister(sf::Packet packet, Client* client)
{
    std::string username, password;
    packet >> username >> password;

    bool insertedUser = SQL_MANAGER.InsertUser(username, password);

    int temp = -1;
    if (insertedUser) temp = 0;
    
    SendAuthenticationResult(client, temp);
}

void NetworkManager::OnReceiveCreateRoom(sf::Packet packet, Client* client)
{
    Room* room;
    bool temp;
    do {
        temp = false;
        room = new Room(); // when created, it creates a random roomCode
        for (auto _room : rooms)
        {
            if (_room->GetRoomCode() == room->GetRoomCode())
            {
                temp = true;
                break;
            }
        }
    } while (temp);
    room->InsertClient(client);
    rooms.push_back(room);
    SendPacketRoomCode(client);
}



void NetworkManager::SendAuthenticationResult(Client* client, int result)
{
    sf::Packet packet;
    packet << SV_AUTH;
    packet << result;
    if (client->GetSocket()->send(packet) == sf::Socket::Status::Done)
    {
        std::cout << "Client " << client->GetID() << " logged with result " << result << std::endl;
        packet.clear();
    }
}

// Sends the socket of other client to the specified client
void NetworkManager::SendPacketIpAdress(Client* client, const sf::TcpSocket& socket)
{
    sf::Packet packet;
    packet << socket;
    if (client->GetSocket()->send(packet) == sf::Socket::Status::Done)
    {
        std::cout << "Client " << client->GetID() << " Received the socket: " 
                  << socket.getRemoteAddress().value().toString() << std::endl;
        packet.clear();
    }
    else
    {
        std::cerr << "Could not send packet to client " << client->GetID() 
                  << " the Socket: " << socket.getRemoteAddress().value().toString() << std::endl;
    }
}

void NetworkManager::SendPacketRoomCode(Client* client)
{
    std::string roomCode = GetRoomCodeOfClient(client);
    if (roomCode == "") return;
    
    sf::Packet packet;
    packet << SV_ROOM_CODE;
    packet << roomCode;

    if (client->GetSocket()->send(packet) == sf::Socket::Status::Done)
    {
        std::cout << "Client " << client->GetID() << " Received the roomCode: "
            << roomCode << std::endl;
        packet.clear();
    }
    else
    {
        std::cerr << "Could not send packet to client " << client->GetID()
            << " the Room Code: " << roomCode << std::endl;
    }
}

void NetworkManager::OnReceiveJoinRoom(sf::Packet packet, Client* client)
{
    std::string roomCode;
    packet >> roomCode;

    if (roomCode == "-1") {
        GetRoomByCode(roomCode)->RemoveClient(client->GetID());
        return;
    }
    Room* currentRoom = GetRoomByCode(roomCode);
    currentRoom->InsertClient(client);
    SendPacketRoomCode(client);

    if (currentRoom->GetIsFull())
    {
        // Set one Client as Host

        // Send him all the Sockets to maintain the gameplay

        // Delete the room

    }
}

void NetworkManager::RegisterNewUserConnection()
{
    Client* newClient = new Client(new sf::TcpSocket());
    int id;
    if (listener.accept(*newClient->GetSocket()) == sf::Socket::Status::Done) // Añadir nuevo cliente HANDSHAKE
    {
        newClient->GetSocket()->setBlocking(false); // Desbloqueamos el socket
        selector.add(*newClient->GetSocket());

        id = GetNextClientId();
        newClient->SetID(id);
        std::cout << "Nueva conexion establecida: " << id << " --> " << newClient->GetIP() << std::endl;
        clients.insert(std::pair<unsigned int, Client*>(id, newClient));

        for (auto room : rooms)
        {
            std::cout << room->GetRoomCode() << " ";
        }
        std::cout << std::endl;
    }
}

void NetworkManager::ReceivePacket(sf::Packet packet, Client* client)
{
    packetType typeSended;
    packet >> typeSended;
    switch (typeSended)
    {
    case LOGIN:
        OnReceiveLogin(packet, client);
        break;
    case REGISTER:
        OnReceiveRegister(packet, client);
        break;
    case CREATE_ROOM:
        OnReceiveCreateRoom(packet, client);
        break;
    case JOIN_ROOM:
        OnReceiveJoinRoom(packet, client);
        break;

    default:
        break;
    }
    
}

void NetworkManager::RemoveClient(Client* client, unsigned int id)
{

    RemoveClientFromRooms(id);
    selector.remove(*client->GetSocket());
    delete client->GetSocket();
    clients.erase(id);

}

void NetworkManager::RemoveClientFromRooms(unsigned int id)
{
    for (auto room : rooms)
    {
        room->RemoveClient(id);
    }
}

void NetworkManager::Init()
{
    closeServer = false;
    if (listener.listen(LISTENER_PORT) != sf::Socket::Status::Done) // Comprbar puerto valido
    {
        std::cerr << "Cannot Listen the port.\nExiting execution with code -1." << std::endl;
        closeServer = true;
    }
    selector.add(listener);
}

bool NetworkManager::Update()
{
    if (selector.wait())
    {
        if (selector.isReady(listener))
        {
            RegisterNewUserConnection();
        }
        else
        {
            Client* client;
            for (auto& pair : clients)
            {
                client = pair.second;

                if (selector.isReady(*client->GetSocket()))
                {
                    sf::Packet packet;
                    if (client->GetSocket()->receive(packet) == sf::Socket::Status::Done)
                    {
                        ReceivePacket(packet, client);
                    }
                    if (client->GetSocket()->receive(packet) == sf::Socket::Status::Disconnected)
                    {
                        RemoveClient(client, pair.first);
                    }
                }
            }
        }
    }
    return true;
}
