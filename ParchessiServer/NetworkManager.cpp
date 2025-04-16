#include "NetworkManager.h"
#include "SQLManager.h"

sf::Packet& operator>>(sf::Packet& _packet, packetType& _type)
{
    int temp;
    _packet >> temp;
    _type = static_cast<packetType>(temp);

    return _packet;
};

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


void NetworkManager::RegisterNewUserConnection()
{
    Client* newClient = new Client(new sf::TcpSocket());
    int id;
    if (listener.accept(*newClient->GetSocket()) == sf::Socket::Status::Done) // Añadir nuevo cliente HANDSHAKE
    {
        newClient->GetSocket()->setBlocking(false); // Desbloqueamos el socket
        selector.add(*newClient->GetSocket());

        id = GetNextClientId();
        std::cout << "Nueva conexion establecida: " << id << " --> " << newClient->GetIP() << std::endl;
        clients.insert(std::pair<unsigned int, Client*>(id, newClient));
    }
}

void NetworkManager::ReceivePacket(sf::Packet packet)
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

    default:
        break;
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
                        ReceivePacket(packet);
                    }
                    if (client->GetSocket()->receive(packet) == sf::Socket::Status::Disconnected)
                    {
                        selector.remove(*client->GetSocket());

                        std::cout << "El cliente se ha desconectado: " << pair.first << std::endl;
                        delete client->GetSocket();
                        clients.erase(pair.first);
                    }
                }
            }
        }
    }
}
