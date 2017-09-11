cmake_minimum_required(VERSION 3.6)
project(Assignment)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lm -lpthread")

set(SOURCE_FILES server.c stuff.h)
#set(SOURCE_FILES client.c stuff.h)
add_executable(Assignment ${SOURCE_FILES})