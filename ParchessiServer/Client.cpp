#include "Client.h"

bool operator==(Client client, Client client2)
{
    if (client.GetID() != client2.GetID() ||
        client.GetIP() != client2.GetIP() ||
        client.GetSocket() != client2.GetSocket())
    {
        return false;
    }
    return true;
}