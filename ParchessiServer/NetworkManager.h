#pragma once

#include "Client.h"
#include <SFML/Network.hpp>
#include <unordered_map>

#define LISTENER_PORT 55000

#define NETWORK_MANAGER NetworkManager::Instance()

enum packetType { LOGIN, REGISTER };



class NetworkManager
{
private:
	NetworkManager() = default;
	~NetworkManager() = default;
	NetworkManager(const NetworkManager&) = delete;
	NetworkManager operator =(const NetworkManager&) = delete;

	sf::TcpListener listener;
	sf::SocketSelector selector;
	std::unordered_map<unsigned int /*id*/, Client*> clients;
	
	bool closeServer = false;

	inline unsigned int GetNextClientId() {
		static unsigned int currentId = 0;
		return currentId++;
	}

	void OnReceiveLogin(sf::Packet packet);
	void OnReceiveRegister(sf::Packet packet);
	void OnReceiveDebug(sf::Packet packet);

	void RegisterNewUserConnection();
	void ReceivePacket(sf::Packet packet);

public:
	inline static NetworkManager& Instance() {
		static NetworkManager instance;
		return instance;
	}
	void Init();
	bool Update();
};