#pragma once
#include <iostream>
#include <SFML/Network.hpp>

class Client
{
private:
    unsigned int id;
    unsigned int sqlID = -1;
    sf::TcpSocket* socket;

public:
    Client(sf::TcpSocket* _socket) : socket(_socket) {}

    inline void SetID(unsigned int _id) { id = _id; }

    inline sf::TcpSocket* GetSocket() { return socket; }
    inline sf::IpAddress GetIP() { return socket->getRemoteAddress().value(); 
    }
    inline unsigned int GetID() { return id; }

    inline void SetSQLID(unsigned int _id) { sqlID = _id; }
    inline unsigned int GetSQLID() { return sqlID; }
};

