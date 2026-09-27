#include "Packet.hpp"
#include "Statistics.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

namespace {

uint64_t nowNanoseconds()
{
    auto now = std::chrono::steady_clock::now();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch());
    return static_cast<uint64_t>(nanos.count());
}

double millisecondsBetween(uint64_t startNanos, uint64_t endNanos)
{
    return static_cast<double>(endNanos - startNanos) / 1000000.0;
}

void printResults(const Statistics& stats)
{
    std::cout << "\n## Results\n\n";
    std::cout << "Packets sent:       " << stats.packetsSent() << "\n";
    std::cout << "Packets received:   " << stats.packetsReceived() << "\n";
    std::cout << "Packet loss:        " << stats.packetLossPercent() << "%\n\n";
    std::cout << "Average RTT:        " << stats.averageRtt() << " ms\n";
    std::cout << "Minimum RTT:        " << stats.minRtt() << " ms\n";
    std::cout << "Maximum RTT:        " << stats.maxRtt() << " ms\n";
    std::cout << "Jitter:             " << stats.jitter() << " ms\n\n";
    std::cout << "Throughput:         " << stats.throughputMbps() << " Mbps\n";
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc != 3 && argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <server-ip> <port> [packet-count]\n";
        return 1;
    }

    const char* serverIp = argv[1];
    int port = std::atoi(argv[2]);
    int packetCount = 100;

    if (argc == 4) {
        packetCount = std::atoi(argv[3]);
    }

    if (port <= 0 || port > 65535) {
        std::cerr << "Invalid port number\n";
        return 1;
    }

    if (packetCount <= 0) {
        std::cerr << "Packet count must be greater than zero\n";
        return 1;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create socket: " << std::strerror(errno) << "\n";
        return 1;
    }

    timeval timeout {};
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        std::cerr << "Failed to set receive timeout: " << std::strerror(errno) << "\n";
        close(sock);
        return 1;
    }

    sockaddr_in serverAddress {};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(static_cast<uint16_t>(port));

    if (inet_pton(AF_INET, serverIp, &serverAddress.sin_addr) != 1) {
        std::cerr << "Invalid server IP address\n";
        close(sock);
        return 1;
    }

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "## UDP Network Quality Test\n\n";
    std::cout << "Server: " << serverIp << ":" << port << "\n";
    std::cout << "Packets to send: " << packetCount << "\n\n";
    std::cout << "Running test...\n\n";

    Statistics stats;
    stats.start();

    for (int i = 1; i <= packetCount; ++i) {
        Packet packet {};
        packet.sequenceNumber = static_cast<uint32_t>(i);
        packet.timestamp = nowNanoseconds();
        std::memset(packet.payload, 'x', sizeof(packet.payload));

        ssize_t bytesSent = sendto(
            sock,
            &packet,
            sizeof(packet),
            0,
            reinterpret_cast<sockaddr*>(&serverAddress),
            sizeof(serverAddress));

        if (bytesSent < 0) {
            std::cerr << "sendto failed for packet " << i << ": "
                      << std::strerror(errno) << "\n";
            stats.recordSent();
            continue;
        }

        stats.recordSent();

        Packet response {};
        sockaddr_in responseAddress {};
        socklen_t responseAddressLength = sizeof(responseAddress);

        ssize_t bytesReceived = recvfrom(
            sock,
            &response,
            sizeof(response),
            0,
            reinterpret_cast<sockaddr*>(&responseAddress),
            &responseAddressLength);

        if (bytesReceived < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cout << "Packet " << i << ": timeout\n";
            } else {
                std::cerr << "recvfrom failed for packet " << i << ": "
                          << std::strerror(errno) << "\n";
            }
            continue;
        }

        if (response.sequenceNumber != static_cast<uint32_t>(i)) {
            std::cout << "Packet " << i << ": unexpected response "
                      << response.sequenceNumber << "\n";
            continue;
        }

        uint64_t receiveTime = nowNanoseconds();
        double rttMs = millisecondsBetween(response.timestamp, receiveTime);
        stats.recordReceived(rttMs, static_cast<std::size_t>(bytesReceived));

        std::cout << "Packet " << response.sequenceNumber << ": "
                  << rttMs << " ms\n";
    }

    stats.finish();
    printResults(stats);

    close(sock);
    return 0;
}
