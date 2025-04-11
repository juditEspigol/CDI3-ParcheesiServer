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
#define SQL_PASSWORD "" //"enti"

#define SQL_DATABASE "TCP_Parchessi"

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

bool InsertUser(sql::Connection* connection, std::string username, std::string password)
{
    std::string hash = bcrypt::generateHash(password);
    std::cout << "HASH 1: " << hash << std::endl;
    try 
    {
        // Query on \SQL\Database Creation Scripts.sql
        std::string query = "INSERT INTO Users (username, password) SELECT * FROM(SELECT ? AS username, ? AS password) AS TemporalTable WHERE NOT EXISTS( SELECT id FROM Users WHERE username = ?)";
        sql::PreparedStatement* statement = connection->prepareStatement(query);

        statement->setString(1, username);
        statement->setString(2, hash);
        statement->setString(3, username);

        int affected_rows = statement->executeUpdate();
        
        delete statement;
        
        if (affected_rows > 0)
        {
            std::cout << "User Inserted Successfully" << std::endl;
            return true;
        }
        std::cerr << "User Not Inserted" << std::endl;

        return false;

    }
    catch (sql::SQLException& e)
    {
        std::cerr << "Error while inserting user: " << e.what() << std::endl;
        return false;
    }
}

int CheckLogin(sql::Connection* connection, std::string username, std::string password)
{
    try
    {
        // Query on \SQL\Database Creation Scripts.sql
        std::string query = "SELECT id, password FROM Users WHERE username = ?";
        sql::PreparedStatement* statement = connection->prepareStatement(query);
        statement->setString(1, username);

        // We obtain all the values returned from the Query
        std::unique_ptr<sql::ResultSet> result(statement->executeQuery());

        delete statement;

        if (result->next())
        {
            std::string storedHash = result->getString("password");

            // Validate hast using bcrypt::validatePassword

            if (bcrypt::validatePassword(password, storedHash)) 
            {
                int userID = result->getInt("id");
                std::cout << "User exists with ID: " << userID << std::endl;
                return userID;
            }
            else
            {
                std::cerr << "Invalid password" << std::endl;
                return -1;
            }
        }
        std::cerr << "Username not found" << std::endl;
        return -1;

    }
    catch (sql::SQLException& e)
    {
        std::cerr << "Error while getting user: " << e.what() << std::endl;
        return -1;
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
    

    if (listener.listen(LISTENER_PORT) != sf::Socket::Status::Done) // Comprbar puerto valido
    {
        std::cerr << "No puedo escuchar el puerto" << std::endl;
        closeServer = true;
    }

    selector.add(listener);

    // TO TEST LOGIN AND REGISTER
    /*InsertUser(connection, "Golondrino", "golomateiro");
    CheckLogin(connection, "Golondrino", "golomateiro");*/


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

                            std::string username, password, hash;
                            switch (typeSended)
                            {
                            case LOGIN:
                                packet >> username; 
                                packet >> password; 
                                int userID = CheckLogin(connection, username, password);
                                // If the userID is -1, login failed

                                break;

                            case REGISTER:
                                packet >> username;
                                packet >> password;
                                bool hasRegistered = InsertUser(connection, username, password);
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

    DisconnectDatabase(connection);
}
