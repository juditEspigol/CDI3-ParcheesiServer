#pragma once
#include "Client.h"

class Room {
private:
	
	bool isFull;
	std::vector<Client*> clients;
	std::string roomCode;

	void GenerateRandomRoomCode(int maxLength);

public:
	Room();
	~Room();

	void InsertClient(Client* client);
	void RemoveClient(unsigned int id);

	inline bool GetIsFull() { return isFull; }
	std::string GetRoomCode() { return roomCode; }


};