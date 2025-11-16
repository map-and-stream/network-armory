#include <gtest/gtest.h>

#include <iostream>
#include <sstream>

#include "factory.h"
#include "network.h"


TEST(SampleTest, BasicAssertion) {
    EXPECT_EQ(1 + 1, 2);
}

TEST(LogTest, InfoOutput) {
    // INetwork* i = NetworkFactory::createNetwork(BackendType::BoostAsio, cfg);
    // testing::internal::CaptureStdout();
    // // l->info("Hello");
    // std::string output = testing::internal::GetCapturedStdout();
    // EXPECT_NE(output.find("[INFO] Hello"), std::string::npos);
}
