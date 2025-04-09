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

#define LISTENER_PORT 55000

#define SQL_IP "127.0.0.1:3306"
#define SQL_USER "root"
#define SQL_PASSWORD "enti"

#define SQL_DATABASE "parchessi"

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

void GetAllUsers(sql::Connection* _connection)
{
    try
    {
        sql::Statement* statement = _connection->createStatement();
        sql::ResultSet* result = statement->executeQuery("SELECT user FROM users");

        std::cout << "Users in the database:" << std::endl;
        while (result->next())
        {
            std::cout << result->getString("user") << std::endl; 
        }

        delete result; 
        delete statement; 
    }
    catch (sql::SQLException e)
    {
        std::cerr << "Could not get user. Error message: " << e.what() << std::endl;
    }
}

void ConnectDatabase(sql::Driver*& _driver, sql::Connection*& _connection)
{
    try
    {
        _driver = get_driver_instance();
        _connection = _driver->connect(SQL_IP, SQL_USER, SQL_PASSWORD);
        _connection->setSchema(SQL_DATABASE);
        std::cout << "Connection Done!" << std::endl; 
    }
    catch (sql::SQLException e)
    {
        std::cerr << "Could not connect to server. Error message: " << e.what() << std::endl; 
    }
}

void DisconnectDatabase(sql::Connection*& _connection)
{
    _connection->close();

    if (_connection->isClosed())
    {
        std::cout << "Connection Close" << std::endl; 
    }
}

void main()
{
    sf::TcpListener listener;
    sf::SocketSelector selector; //Funciona como un vector, adds y removes // es para optimizar recursos, no te dice quien te ha dicho algo

    std::unordered_map<unsigned int /*id*/, Client*> clients;
    Client* newClient;
    unsigned int idClient = 0;

    bool closeServer = false;

    // DATABASE
    sql::Driver* driver; 
    sql::Connection* connection; 
    ConnectDatabase(driver, connection);
    // 

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

                    GetAllUsers(connection);
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

                            std::string username, password, hash;
                            switch (typeSended)
                            {
                            case LOGIN:
                                packet >> username; 
                                packet >> password; 
                                hash = bcrypt::generateHash(password);
                                std::cout << "You have login as: " << username << ", with password: " << hash << std::endl;
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

    DisconnectDatabase(connection);
}
