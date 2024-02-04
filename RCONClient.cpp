#include "RCONClient.h"
#include <stdexcept>
#include <memory>

RCONClient::RCONClient() : rconSocket(INVALID_SOCKET), server{} {
    WSADATA wsaData;
    int wsaStartupError = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaStartupError != 0) {
        throw std::runtime_error("WSAStartup failed with error: " + std::to_string(wsaStartupError));
    }
}

RCONClient::~RCONClient() {
    if (rconSocket != INVALID_SOCKET) {
        closesocket(rconSocket);
    }
    WSACleanup();
}

bool RCONClient::Connect(const std::string& ipAddress, int port, const std::string& password) {
    if (rconSocket != INVALID_SOCKET) {
        closesocket(rconSocket);
        rconSocket = INVALID_SOCKET;
    }

    rconSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (rconSocket == INVALID_SOCKET) {
        return false;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if (inet_pton(AF_INET, ipAddress.c_str(), &server.sin_addr) <= 0) {
        closesocket(rconSocket);
        rconSocket = INVALID_SOCKET;
        return false;
    }

    if (connect(rconSocket, (struct sockaddr*)&server, sizeof(server)) < 0) {
        closesocket(rconSocket);
        rconSocket = INVALID_SOCKET;
        return false;
    }

    RCONPacket authPacket;
    initPacket(authPacket, 1, 3, password);
    if (!sendPacket(authPacket)) {
        closesocket(rconSocket);
        rconSocket = INVALID_SOCKET;
        return false;
    }

    RCONPacket response;
    if (!receivePacket(response) || response.id == -1) {
        closesocket(rconSocket);
        rconSocket = INVALID_SOCKET;
        return false;
    }

    return true;
}

void RCONClient::initPacket(RCONPacket& packet, int id, int type, const std::string& command) {
    packet.id = id;
    packet.type = type;

    int commandBytes = static_cast<int>(command.size());
    if (commandBytes >= sizeof(packet.body)) {
        throw std::runtime_error("Command too long");
    }

    memcpy(packet.body, command.c_str(), commandBytes);

    packet.body[commandBytes] = '\0';

    packet.size = sizeof(packet.id) + sizeof(packet.type) + commandBytes + 2;
}

bool RCONClient::sendPacket(RCONPacket& packet) {
    int totalSize = sizeof(packet.size) + packet.size;
    if (totalSize > sizeof(RCONPacket)) {
        throw std::runtime_error("Packet size too large");
    }

    std::unique_ptr<char[]> buffer(new char[totalSize]);
    memcpy(buffer.get(), &packet.size, sizeof(packet.size));
    memcpy(buffer.get() + sizeof(packet.size), &packet.id, sizeof(packet.id));
    memcpy(buffer.get() + sizeof(packet.size) + sizeof(packet.id), &packet.type, sizeof(packet.type));
    memcpy(buffer.get() + sizeof(packet.size) + sizeof(packet.id) + sizeof(packet.type), packet.body, strlen(packet.body) + 2);

    int sentBytes = send(rconSocket, buffer.get(), totalSize, 0);
    return sentBytes == totalSize;
}

bool RCONClient::receivePacket(RCONPacket& packet) {
    std::unique_ptr<char[]> buffer(new char[4096]);
    int received = recv(rconSocket, buffer.get(), 4096, 0);
    if (received > 0) {
        memset(packet.body, 0, sizeof(packet.body));

        memcpy(&packet.size, buffer.get(), sizeof(packet.size));
        memcpy(&packet.id, buffer.get() + sizeof(packet.size), sizeof(packet.id));
        memcpy(&packet.type, buffer.get() + sizeof(packet.size) + sizeof(packet.id), sizeof(packet.type));

        int headerSize = sizeof(packet.size) + sizeof(packet.id) + sizeof(packet.type);
        int bodySize = received - headerSize;
        if (bodySize > sizeof(packet.body) - 1) {
            bodySize = sizeof(packet.body) - 1;
        }

        memcpy(packet.body, buffer.get() + headerSize, bodySize);

        packet.body[bodySize] = '\0';

        return true;
    }
    return false;
}

bool RCONClient::SendCommand(const std::string& command, std::string& response) {
    if (rconSocket == INVALID_SOCKET) {
        throw std::runtime_error("Not connected to RCON server.");
    }

    RCONPacket commandPacket;
    initPacket(commandPacket, 2, 2, command);
    if (!sendPacket(commandPacket)) {
        return false;
    }

    RCONPacket responsePacket;
    if (!receivePacket(responsePacket)) {
        return false;
    }

    response = std::string(responsePacket.body, strlen(responsePacket.body));
    return true;
}
