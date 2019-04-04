cmake_minimum_required(VERSION 2.8)

set(CMAKE_CSS_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++17 -Wall -pedantic -Wextra -Wconversion -fexceptions")

file(GLOB SRC "*.cpp")
add_executable(ATM ${SRC})
