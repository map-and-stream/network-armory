# Find the installed GoogleTest package
find_package(GTest REQUIRED)

# Enable testing
enable_testing()

# Create your test target (use correct relative paths)
add_executable(network_test
    test_network.cpp

)

# Link against your project library (LIB_ALIAS) and GoogleTest, pthread
target_link_libraries(network_test 
    PRIVATE 
        ${LIB_ALIAS}
        GTest::gtest
        GTest::gtest_main
        pthread
)

# Optional: register the test with CTest
add_test(NAME network_test COMMAND network_test)
