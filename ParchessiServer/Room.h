#pragma once
#include "Client.h"

#define MAX_ROOM_SIZE 4
#define MAX_CHARS_CODE 1

class Room {
private:
	
	bool isFull;
	std::vector<Client*> clients;
	std::string roomCode;

	void GenerateRandomRoomCode(int maxLength);
	int currentId = 0;

public:
	Room();
	~Room();

	inline unsigned int GetNextClientId() {
		return currentId++;
	}

	void InsertClient(Client* client);
	void RemoveClient(unsigned int id);

	inline bool GetIsEmpty() { return clients.size() == 0; }
	inline bool GetIsFull() { return isFull; }
	std::string GetRoomCode() { return roomCode; }
	bool HasClient(Client* client);

	std::vector<Client*> GetClients() { return clients; };

};