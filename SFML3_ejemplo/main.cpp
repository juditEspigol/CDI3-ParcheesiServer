#include <sfml/Network.hpp>
#include <iostream>
#include <string>

#define LISTENER_PORT 55000

enum tipoPaquete { HANDSHAKE, POLLA, BANANA };

int main()
{
	sf::TcpListener listener;
	sf::TcpSocket client;

	sf::SocketSelector selector;
	
	bool closeServer = false;

	std::vector<sf::TcpSocket*> clients;
	sf::TcpSocket* newClient;
 
	if (listener.listen(LISTENER_PORT) != sf::Socket::Status::Done)
	{
		std::cout << "CAGASTE PUTA" << std::endl;
		return 1;
	}
	selector.add(listener);
	std::cout << "Server abierto" << std::endl;
	while (!closeServer)
	{
		if (selector.wait())
		{
			if (selector.isReady(listener))
			{
				newClient = new sf::TcpSocket();
				if (listener.accept(*newClient) == sf::Socket::Status::Done)
				{
					newClient->setBlocking(false);
					selector.add(*newClient);
					clients.push_back(newClient);
					std::cout << "Nueva conexion establecida: " << newClient->getRemoteAddress().value() << std::endl;
				}
				else
				{
					for (int i = 0; i < clients.size(); i++)
					{
						if (selector.isReady(*clients[i]))
						{
							sf::Packet packet;

							if (clients[i]->receive(packet) == sf::Socket::Status::Done)
							{
								std::string message;
								packet >> message;
							}

							if (clients[i]->receive(packet) == sf::Socket::Status::Disconnected)
							{
								selector.remove(*clients[i]);
								delete clients[i];
								clients.erase(clients.begin() + i);
								i--;
							}
						}
					}
				}
			}
		}
		/*if (selector.wait())
		{
			if (selector.isReady(listener))
			{
				std::cout << "Pinga" << std::endl;
				newClient = new sf::TcpSocket();
				if (listener.accept(*newClient) == sf::Socket::Status::Done)
				{
					newClient->setBlocking(false);
					selector.add(*newClient);
					clients.push_back(newClient);
				}
			}


		}*/
		/*std::cout << "Esperando conexiones.." << std::endl;
		if (listener.accept(client) == sf::Socket::Status::Done)
		{
			std::cout << "CLIENTE PUTERO SE LLAMA " << client.getRemoteAddress().value() << std::endl;
		
			sf::Packet packet;
			std::string message = "Tu madre la tiene gordisima";

			packet << message;
			
			if (client.send(packet) == sf::Socket::Status::Done)
			{
				std::cout << "Mensaje: " << message << std::endl;
				packet.clear();
			}
			else {
				std::cerr << "CAGASTE PUTA 3: NO ME LO CREO " << std::endl;
			}
			
			
		
		}
		else
		{
			std::cout << "Cagaste puta 2, el reencuentro del jedi con su madre la gorda" << std::endl;
		}*/
	}
	
	return 0;
}