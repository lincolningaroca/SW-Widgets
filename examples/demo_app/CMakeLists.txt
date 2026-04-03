cmake_minimum_required(VERSION 3.16)

project(SW_Demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(SW_Demo main.cpp)

target_link_libraries(SW_Demo
    PRIVATE
        Qt6::Widgets
        SW_Widgets
)