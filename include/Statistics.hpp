#ifndef STATISTICS_HPP
#define STATISTICS_HPP

#include <chrono>
#include <cstddef>
#include <vector>

class Statistics {
public:
    void start();
    void finish();

    void recordSent();
    void recordReceived(double rttMs, std::size_t bytesReceived);

    int packetsSent() const;
    int packetsReceived() const;
    std::size_t totalBytesReceived() const;

    double packetLossPercent() const;
    double averageRtt() const;
    double minRtt() const;
    double maxRtt() const;
    double jitter() const;
    double throughputMbps() const;

private:
    int packetsSent_ = 0;
    int packetsReceived_ = 0;
    std::size_t totalBytesReceived_ = 0;
    std::vector<double> rttValues_;
    std::chrono::steady_clock::time_point startTime_;
    std::chrono::steady_clock::time_point endTime_;
    bool hasStarted_ = false;
    bool hasFinished_ = false;
};

#endif
