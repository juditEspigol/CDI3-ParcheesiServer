#pragma once
#include <iostream>
#include <SFML/Network.hpp>

class Client
{
private:
    sf::TcpSocket* socket;
    std::string name;

public:
    Client(sf::TcpSocket* _socket) : socket(_socket) {}

    sf::TcpSocket* GetSocket() { return socket; }
    sf::IpAddress GetIP()
    {
        return socket->getRemoteAddress().value();
    }
};