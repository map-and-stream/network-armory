# Find the installed GoogleTest package
find_package(GTest REQUIRED)

# Enable testing
enable_testing()

# Create your test target
add_executable(network_test
    test_network.cpp
    tcp_test.cpp
    udp_test.cpp
)

# Link against GoogleTest and pthread
target_link_libraries(network_test 
PRIVATE 
    ${LIB_ALIAS}
    GTest::gtest
    GTest::gtest_main
    pthread
)
