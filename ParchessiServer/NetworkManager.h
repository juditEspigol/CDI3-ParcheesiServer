#pragma once

#include "Room.h"
#include <SFML/Network.hpp>
#include <unordered_map>

#define LISTENER_PORT 55000

#define NETWORK_MANAGER NetworkManager::Instance()

enum packetType { 
	LOGIN,
	REGISTER, 
	CREATE_ROOM, 
	JOIN_ROOM, 
	SV_AUTH,
	SV_ROOM_CODE, 
	SV_SOCKET
};

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


	inline unsigned int GetNextClientId() {
		static unsigned int currentId = 0;
		return currentId++;
	}

	// Private Functions
	Room* GetRoomByCode(std::string roomCode);
	std::string GetRoomCodeOfClient(Client* client);

	void OnReceiveLogin(sf::Packet packet, Client* client);
	void OnReceiveRegister(sf::Packet packet, Client* client);
	void OnReceiveJoinRoom(sf::Packet packet, Client* client);
	void OnReceiveCreateRoom(sf::Packet packet, Client* client);

	void SendAuthenticationResult(Client* client, int result);
	void SendPacketIpAdress(Client* client, const sf::TcpSocket& socket);
	void SendPacketRoomCode(Client* client);

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