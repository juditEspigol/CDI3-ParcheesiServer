// Server!!!!

#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include "Client.h"

#define LISTENER_PORT 55000

enum packetType { LOGIN, REGISTER };

void HandShake(sf::Packet _data)
{
    std::string receivedMessage;
    _data >> receivedMessage; // Sacar el mensaje del packet

    std::cout << "Mensaje recivido del servidor: " << receivedMessage << std::endl;
}

sf::Packet& operator>>(sf::Packet& _packet, packetType& _type)
{
    int temp;
    _packet >> temp;
    _type = static_cast<packetType>(temp);

    return _packet;
};

void main()
{
    sf::TcpListener listener;
    sf::SocketSelector selector; //Funciona como un vector, adds y removes // es para optimizar recursos, no te dice quien te ha dicho algo

    std::unordered_map<unsigned int /*id*/, Client*> clients;
    Client* newClient;
    unsigned int idClient = 0;

    bool closeServer = false;

    if (listener.listen(LISTENER_PORT) != sf::Socket::Status::Done) // Comprbar puerto valido
    {
        std::cerr << "No puedo escuchar el puerto" << std::endl;
        closeServer = true;
    }

    selector.add(listener);

    while (!closeServer)
    {
        if (selector.wait())
        {
            if (selector.isReady(listener))
            {

                newClient = new Client(new sf::TcpSocket());
                if (listener.accept(*newClient->GetSocket()) == sf::Socket::Status::Done) // Añadir nuevo cliente HANDSHAKE
                {
                    newClient->GetSocket()->setBlocking(false); // Desbloqueamos el socket
                    selector.add(*newClient->GetSocket());

                    std::cout << "Nueva conexion establecida: " << idClient << " --> " << newClient->GetIP() << std::endl;
                    clients.insert(std::pair<unsigned int, Client*>(idClient++, newClient));
                }
            }
            else // el cliente ya esta conectado, revisamos todos los clientes si tiene información nueva
            {
                Client* client;
                for (auto& pair : clients)
                {
                    client = pair.second;

                    if (selector.isReady(*client->GetSocket())) // cambio en los clientes
                    {
                        sf::Packet packet;
                        if (client->GetSocket()->receive(packet) == sf::Socket::Status::Done)
                        {
                            packetType typeSended; 
                            packet >> typeSended; 

                            std::string username, password;
                            switch (typeSended)
                            {
                            case LOGIN:
                                packet >> username; 
                                packet >> password; 

                                std::cout << "You have login as: " << username << ", with password: " << password << std::endl;
                                break;
                            case REGISTER:

                                packet >> username;
                                packet >> password;

                                std::cout << "You have registered with: " << username << ", with password: " << password << std::endl;

                                break;
                            default:
                                break;
                            }
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
}
