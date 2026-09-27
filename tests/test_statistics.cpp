#include "Statistics.hpp"

#include <gtest/gtest.h>

TEST(StatisticsTest, StartsWithZeroValues)
{
    Statistics stats;

    EXPECT_EQ(stats.packetsSent(), 0);
    EXPECT_EQ(stats.packetsReceived(), 0);
    EXPECT_EQ(stats.totalBytesReceived(), 0u);
    EXPECT_DOUBLE_EQ(stats.packetLossPercent(), 0.0);
    EXPECT_DOUBLE_EQ(stats.averageRtt(), 0.0);
    EXPECT_DOUBLE_EQ(stats.minRtt(), 0.0);
    EXPECT_DOUBLE_EQ(stats.maxRtt(), 0.0);
    EXPECT_DOUBLE_EQ(stats.jitter(), 0.0);
    EXPECT_DOUBLE_EQ(stats.throughputMbps(), 0.0);
}

TEST(StatisticsTest, CalculatesPacketLoss)
{
    Statistics stats;
    stats.recordSent();
    stats.recordSent();
    stats.recordSent();
    stats.recordSent();
    stats.recordReceived(10.0, 1000);

    EXPECT_DOUBLE_EQ(stats.packetLossPercent(), 75.0);
}

TEST(StatisticsTest, CalculatesAverageRtt)
{
    Statistics stats;
    stats.recordReceived(10.0, 1000);
    stats.recordReceived(20.0, 1000);
    stats.recordReceived(30.0, 1000);

    EXPECT_DOUBLE_EQ(stats.averageRtt(), 20.0);
}

TEST(StatisticsTest, CalculatesMinimumRtt)
{
    Statistics stats;
    stats.recordReceived(30.0, 1000);
    stats.recordReceived(12.5, 1000);
    stats.recordReceived(18.0, 1000);

    EXPECT_DOUBLE_EQ(stats.minRtt(), 12.5);
}

TEST(StatisticsTest, CalculatesMaximumRtt)
{
    Statistics stats;
    stats.recordReceived(30.0, 1000);
    stats.recordReceived(12.5, 1000);
    stats.recordReceived(44.0, 1000);

    EXPECT_DOUBLE_EQ(stats.maxRtt(), 44.0);
}

TEST(StatisticsTest, CalculatesSimplifiedJitter)
{
    Statistics stats;
    stats.recordReceived(20.0, 1000);
    stats.recordReceived(24.0, 1000);
    stats.recordReceived(21.0, 1000);
    stats.recordReceived(30.0, 1000);

    EXPECT_DOUBLE_EQ(stats.jitter(), 16.0 / 3.0);
}

TEST(StatisticsTest, JitterNeedsAtLeastTwoRttValues)
{
    Statistics stats;
    stats.recordReceived(20.0, 1000);

    EXPECT_DOUBLE_EQ(stats.jitter(), 0.0);
}

TEST(StatisticsTest, TracksReceivedBytes)
{
    Statistics stats;
    stats.recordReceived(10.0, 1000);
    stats.recordReceived(20.0, 500);

    EXPECT_EQ(stats.totalBytesReceived(), 1500u);
}

TEST(StatisticsTest, CalculatesThroughput)
{
    Statistics stats;
    stats.start();
    stats.recordReceived(10.0, 100000);
    stats.finish();

    EXPECT_GE(stats.throughputMbps(), 0.0);
}
