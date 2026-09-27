#include "Statistics.hpp"

void Statistics::start()
{
    startTime_ = std::chrono::steady_clock::now();
    endTime_ = startTime_;
    hasStarted_ = true;
    hasFinished_ = false;
}

void Statistics::finish()
{
    endTime_ = std::chrono::steady_clock::now();
    hasFinished_ = true;
}

void Statistics::recordSent()
{
    ++packetsSent_;
}

void Statistics::recordReceived(double rttMs, std::size_t bytesReceived)
{
    ++packetsReceived_;
    totalBytesReceived_ += bytesReceived;
    rttValues_.push_back(rttMs);
}

int Statistics::packetsSent() const
{
    return packetsSent_;
}

int Statistics::packetsReceived() const
{
    return packetsReceived_;
}

std::size_t Statistics::totalBytesReceived() const
{
    return totalBytesReceived_;
}

double Statistics::packetLossPercent() const
{
    if (packetsSent_ == 0) {
        return 0.0;
    }

    int lostPackets = packetsSent_ - packetsReceived_;
    return static_cast<double>(lostPackets) / packetsSent_ * 100.0;
}

double Statistics::averageRtt() const
{
    if (rttValues_.empty()) {
        return 0.0;
    }

    double total = 0.0;
    for (double rtt : rttValues_) {
        total += rtt;
    }

    return total / rttValues_.size();
}

double Statistics::minRtt() const
{
    if (rttValues_.empty()) {
        return 0.0;
    }

    double minimum = rttValues_[0];
    for (double rtt : rttValues_) {
        if (rtt < minimum) {
            minimum = rtt;
        }
    }

    return minimum;
}

double Statistics::maxRtt() const
{
    if (rttValues_.empty()) {
        return 0.0;
    }

    double maximum = rttValues_[0];
    for (double rtt : rttValues_) {
        if (rtt > maximum) {
            maximum = rtt;
        }
    }

    return maximum;
}

double Statistics::jitter() const
{
    if (rttValues_.size() < 2) {
        return 0.0;
    }

    double totalDifference = 0.0;
    for (std::size_t i = 1; i < rttValues_.size(); ++i) {
        double difference = rttValues_[i] - rttValues_[i - 1];
        if (difference < 0.0) {
            difference = -difference;
        }
        totalDifference += difference;
    }

    return totalDifference / (rttValues_.size() - 1);
}

double Statistics::throughputMbps() const
{
    if (!hasStarted_) {
        return 0.0;
    }

    auto finalTime = hasFinished_ ? endTime_ : std::chrono::steady_clock::now();
    std::chrono::duration<double> duration = finalTime - startTime_;
    double seconds = duration.count();

    if (seconds <= 0.0) {
        return 0.0;
    }

    return (static_cast<double>(totalBytesReceived_) * 8.0) / seconds / 1000000.0;
}
