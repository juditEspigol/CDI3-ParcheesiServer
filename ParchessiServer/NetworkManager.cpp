#include "NetworkManager.h"
#include "SQLManager.h"

sf::Packet& operator>>(sf::Packet& _packet, packetType& _type)
{
    int temp;
    _packet >> temp;
    _type = static_cast<packetType>(temp);

    return _packet;
};

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

void NetworkManager::OnReceiveLogin(sf::Packet packet)
{
    std::string username, password;
    packet >> username >> password;
    int num = SQL_MANAGER.CheckLogin(username, password);
}

void NetworkManager::OnReceiveRegister(sf::Packet packet)
{
    std::string username, password;
    packet >> username >> password;
    bool insertedUser = SQL_MANAGER.InsertUser(username, password);
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
}
void NetworkManager::OnReceiveJoinRoom(sf::Packet packet, Client* client)
{
    std::string roomCode;
    packet >> roomCode;

    if (roomCode == "-1") {
        GetRoomByCode(roomCode)->RemoveClient(client->GetID());
        return;
    }
    GetRoomByCode(roomCode)->InsertClient(client);
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
        OnReceiveLogin(packet);
        break;
    case REGISTER:
        OnReceiveRegister(packet);
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
