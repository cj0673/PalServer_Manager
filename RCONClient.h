#ifndef RCONCLIENT_H
#define RCONCLIENT_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

struct RCONPacket {
    int size;
    int id;
    int type;
    char body[4096];
};

class RCONClient {
public:
    RCONClient();
    ~RCONClient();

    bool Connect(const std::string& ipAddress, int port, const std::string& password);
    bool SendCommand(const std::string& command, std::string& response);

private:
    SOCKET rconSocket;
    struct sockaddr_in server;

    void initPacket(RCONPacket& packet, int id, int type, const std::string& command);
    bool sendPacket(RCONPacket& packet);
    bool receivePacket(RCONPacket& packet);
};

#endif // RCONCLIENT_H
