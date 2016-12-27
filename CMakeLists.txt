cmake_minimum_required(VERSION 3.6)
project(APex4)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp src/Point.h src/Point.cpp src/BFS.cpp src/BFS.h src/Node.cpp src/Node.h
        src/Grid.cpp src/Grid.h src/Driver.cpp src/Driver.h src/Passenger.h src/Passenger.cpp
        src/TripInfo.h src/TripInfo.cpp src/TaxiCenter.h src/TaxiCenter.cpp src/Cab.h src/Cab.cpp
        src/StandardCab.h src/StandardCab.cpp src/LuxuryCab.h src/LuxuryCab.cpp src/MainFlow.h
        src/MainFlow.cpp src/Color.h src/Manufacturer.h src/MaritalStatus.h)
add_executable(APex4 ${SOURCE_FILES})