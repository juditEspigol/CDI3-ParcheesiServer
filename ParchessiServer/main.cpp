// Server!!!!

#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include "bcrypt.h"
#include "Client.h"

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

#include "SQLManager.h"

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

    // DATABASE
    SQLManager::Instance().ConnectDatabase();
    

    if (listener.listen(LISTENER_PORT) != sf::Socket::Status::Done) // Comprbar puerto valido
    {
        std::cerr << "No puedo escuchar el puerto" << std::endl;
        closeServer = true;
    }

    selector.add(listener);

    // TO TEST LOGIN AND REGISTER
    /*InsertUser(connection, "Judith", "Espigol");
    CheckLogin(connection, "Judith", "Espigol");*/

    int userID;
    bool hasRegistered;
    std::string username, password, hash;
    packetType typeSended;

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
                            packet >> typeSended/* >> username >> password*/;
                            //std::cout << "TypeSended: " << typeSended << " Username: " << username << "  Password: " << password << std::endl;
                            switch (typeSended)
                            {
                            case LOGIN:
                                packet >> username; 
                                packet >> password; 
                                std::cout << "Username: " << username << "  Password: " << password << std::endl;
                                userID = SQLManager::Instance().CheckLogin(username, password);
                                // If the userID is -1, login failed

                                break;

                            case REGISTER:
                                packet >> username;
                                packet >> password;
                                std::cout << "Username: " << username << "  Password: " << password << std::endl;
                                SQLManager::Instance().InsertUser(username, password);
                                // if false, register failed

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

    SQLManager::Instance().DisconnectDatabase();
}
