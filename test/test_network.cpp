#include <gtest/gtest.h>

#include <iostream>
#include <sstream>


TEST(SampleTest, BasicAssertion) {
    EXPECT_EQ(1 + 1, 2);
}

TEST(LogTest, InfoOutput) {
    // Start capturing stdout
    testing::internal::CaptureStdout();

    // Simulate your logger output
    std::cout << "[INFO] Hello";

    // Stop capturing
    std::string output = testing::internal::GetCapturedStdout();

    // Verify the output
    EXPECT_NE(output.find("[INFO] Hello"), std::string::npos);
}
