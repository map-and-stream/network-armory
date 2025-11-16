#include <gtest/gtest.h>

#include <iostream>
#include <sstream>

#include "factory.h"
#include "network.h"


TEST(SampleTest, BasicAssertion) {
    EXPECT_EQ(1 + 1, 2);
}

TEST(LogTest, InfoOutput) {
    NetworkConfig cfg;


    INetwork* i = NetworkFactory::createNetwork(NetworkType::standard, cfg);
    testing::internal::CaptureStdout();
    // l->info("Hello");
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("[INFO] Hello"), std::string::npos);
}
