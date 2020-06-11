cmake_minimum_required (VERSION 3.13)
SET (CMAKE_BUILD_TYPE "Release")
SET (CMAKE_CXX_FLAGS "-s -std=c++14 -static-libgcc -static-libstdc++ -static")
SET (PROJECT SIServer)

project(${PROJECT})




SET (HEADERS
        src/server.h
        src/connection.h
        src/connectionclient.h
	src/protocol.h
        src/connectionremoteemulator.h
	)

SET (SOURCES
	src/main.cpp
        src/server.cpp
        src/connection.cpp
        src/connectionclient.cpp
        src/connectionremoteemulator.cpp
	)

add_executable (${PROJECT} ${HEADERS} ${SOURCES})

if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    target_link_libraries (SIServer ws2_32.lib mswsock.lib PowrProf.lib Winmm.lib)
endif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)

if(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
    target_link_libraries (SIServer ${LIBS} pthread)
endif(${CMAKE_SYSTEM_NAME} STREQUAL Linux)

target_compile_options (${PROJECT} PUBLIC -fPIC)

install (TARGETS SIServer DESTINATION ${CMAKE_BINARY_DIR}/bin)
