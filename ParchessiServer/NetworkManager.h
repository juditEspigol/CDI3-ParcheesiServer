#pragma once

#include "Room.h"
#include <SFML/Network.hpp>
#include <unordered_map>

#define LISTENER_PORT 55000

#define NETWORK_MANAGER NetworkManager::Instance()

enum packetType { LOGIN, REGISTER, CREATE_ROOM, JOIN_ROOM };

class NetworkManager
{
private:
	// Singleton
	NetworkManager() = default;
	~NetworkManager() = default;
	NetworkManager(const NetworkManager&) = delete;
	NetworkManager operator =(const NetworkManager&) = delete;

	// Variables
	sf::TcpListener listener;
	sf::SocketSelector selector;
	std::unordered_map<unsigned int /*id*/, Client*> clients;
	std::vector<Room*> rooms;

	bool closeServer = false;


	// Private Functions
	inline unsigned int GetNextClientId() {
		static unsigned int currentId = 0;
		return currentId++;
	}

	Room* GetRoomByCode(std::string roomCode);

	void OnReceiveLogin(sf::Packet packet);
	void OnReceiveRegister(sf::Packet packet);
	void OnReceiveJoinRoom(sf::Packet packet, Client* client);
	void OnReceiveCreateRoom(sf::Packet packet, Client* client);

	void RegisterNewUserConnection();
	void ReceivePacket(sf::Packet packet, Client* client);

	void RemoveClient(Client* client, unsigned int id);
	void RemoveClientFromRooms(unsigned int id);

public:
	inline static NetworkManager& Instance() {
		static NetworkManager instance;
		return instance;
	}
	void Init();
	bool Update();
};