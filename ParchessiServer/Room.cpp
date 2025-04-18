#include "Room.h"

#define MAX_ROOM_SIZE 4
#define MAX_CHARS_CODE 4

Room::Room()
{
	GenerateRandomRoomCode(MAX_CHARS_CODE);
}

Room::~Room()
{

}

void Room::GenerateRandomRoomCode(int maxLength)
{
	roomCode = "";
	roomCode.reserve(maxLength);

	for (int i = 0; i < maxLength; ++i)
	{
		// 'A' + (0..25)
		char letra = 'A' + (std::rand() % 26);
		roomCode.push_back(letra);
	}

}

void Room::InsertClient(Client* client)
{
	bool temp = false;
	for (auto _client : clients)
	{
		if (_client == client)
		{
			temp = true;
		}
	}
	if (temp) return;
	clients.push_back(client);
	std::cout << "Inserted User " << client->GetID() << " on Room " << roomCode << std::endl;
	isFull = clients.size() >= MAX_ROOM_SIZE;
}

void Room::RemoveClient(unsigned int id)
{
	int index = 0;
	for (int i = 0; i <= clients.size(); i++)
	{
		if (id == clients[i]->GetID())
		{
			std::cout << "Removed User " << id << " from Room " << roomCode << std::endl;
			clients.erase(clients.begin() + i);
			break;
		}
	}
	isFull = clients.size() >= MAX_ROOM_SIZE;
}
