set(LIBRARY_NAME network_armory)

file(GLOB_RECURSE CONFIGURE_DEPENDS NETWORK_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
set(NETWORK_SOURCES
    boost/serialconnection.cpp 
    boost/tcpconnection.cpp 
    boost/tcpserver.cpp 
    boost/udpconnection.cpp 
)

# Find Boost and Asio
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

add_library(${LIBRARY_NAME} STATIC 
${NETWORK_SOURCES}
${NETWORK_HEADERS}
)

# Link Boost::asio (header-only), Boost::system, and threads
target_link_libraries(${LIBRARY_NAME} PUBLIC
    Boost::system
    Threads::Threads
)

# Optionally (if using Boost headers, e.g., for boost/asio.hpp)
target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIRS})

# Keep headers associated with this target for IDEs, but do not propagate
target_sources(${LIBRARY_NAME} PRIVATE ${NETWORK_HEADERS})

# Convenient namespaced alias for downstream targets
add_library(${LIB_ALIAS} ALIAS ${LIBRARY_NAME})
