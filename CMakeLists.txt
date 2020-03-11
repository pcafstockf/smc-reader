cmake_minimum_required(VERSION 3.15)
project(smc_reader)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)

add_executable(smc_reader
	src/apple-smc-reader.cpp
	src/apple-smc-reader.h
	src/main.cpp)
