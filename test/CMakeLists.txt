# Find the installed GoogleTest package
find_package(GTest REQUIRED)

# Enable testing
enable_testing()

# Create your test target
add_executable(network_test
    test_network.cpp
)


# Add include directories so headers like server/posix/tcp_server.h can be found
target_include_directories(network_test PRIVATE
    ${CMAKE_SOURCE_DIR}
)

# Link against your project library (LIB_ALIAS) and GoogleTest, pthread
target_link_libraries(network_test 
    PRIVATE 
        ${LIB_ALIAS}
        GTest::gtest
        GTest::gtest_main
        pthread
)

# Register the test with CTest
add_test(NAME network_test COMMAND network_test)
