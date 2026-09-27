#include "Packet.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }

    int port = std::atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        std::cerr << "Invalid port number\n";
        return 1;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create socket: " << std::strerror(errno) << "\n";
        return 1;
    }

    sockaddr_in serverAddress {};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(static_cast<uint16_t>(port));

    if (bind(sock, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) < 0) {
        std::cerr << "Failed to bind socket: " << std::strerror(errno) << "\n";
        close(sock);
        return 1;
    }

    std::cout << "UDP server listening on port " << port << "\n";

    while (true) {
        Packet packet {};
        sockaddr_in clientAddress {};
        socklen_t clientAddressLength = sizeof(clientAddress);

        ssize_t bytesReceived = recvfrom(
            sock,
            &packet,
            sizeof(packet),
            0,
            reinterpret_cast<sockaddr*>(&clientAddress),
            &clientAddressLength);

        if (bytesReceived < 0) {
            std::cerr << "recvfrom failed: " << std::strerror(errno) << "\n";
            continue;
        }

        ssize_t bytesSent = sendto(
            sock,
            &packet,
            static_cast<std::size_t>(bytesReceived),
            0,
            reinterpret_cast<sockaddr*>(&clientAddress),
            clientAddressLength);

        if (bytesSent < 0) {
            std::cerr << "sendto failed: " << std::strerror(errno) << "\n";
            continue;
        }

        std::cout << "Echoed packet " << packet.sequenceNumber << "\n";
    }

    close(sock);
    return 0;
}
