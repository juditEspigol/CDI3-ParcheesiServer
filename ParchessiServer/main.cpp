#include "SQLManager.h"
#include "NetworkManager.h"


//void HandShake(sf::Packet _data)
//{
//    std::string receivedMessage;
//    _data >> receivedMessage; // Sacar el mensaje del packet
//
//    std::cout << "Mensaje recivido del servidor: " << receivedMessage << std::endl;
//}

void main()
{
    bool closeServer = false;

    // DATABASE
    SQL_MANAGER.ConnectDatabase();
    NETWORK_MANAGER.Init();

    while (!closeServer)
    {
        NETWORK_MANAGER.Update();
    }

    SQL_MANAGER.DisconnectDatabase();
}
