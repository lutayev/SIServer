cmake_minimum_required (VERSION 3.13)
SET (CMAKE_BUILD_TYPE "Debug")
SET (CMAKE_CXX_FLAGS "-g -std=c++14")
SET (PROJECT SIServer)

project(${PROJECT})


find_package (Qt5Core REQUIRED)

qt5_wrap_cpp (MOC_SOURCES ${HEADERS})

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

add_executable (${PROJECT} ${HEADERS} ${MOC_SOURCES} ${SOURCES})

if(${CMAKE_SYSTEM_NAME} STREQUAL Windows)
    target_link_libraries (SIServer Qt5::Core ws2_32.lib mswsock.lib PowrProf.lib)
endif(${CMAKE_SYSTEM_NAME} STREQUAL Windows)

if(${CMAKE_SYSTEM_NAME} STREQUAL Linux)
    target_link_libraries (SIServer Qt5::Core ${LIBS} pthread)
endif(${CMAKE_SYSTEM_NAME} STREQUAL Linux)

target_compile_options (${PROJECT} PUBLIC -fPIC)

install (TARGETS SIServer DESTINATION ${CMAKE_BINARY_DIR}/bin)
