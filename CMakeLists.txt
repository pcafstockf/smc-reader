cmake_minimum_required(VERSION 3.15)
project(smc_reader)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)

add_executable(smc_reader
	src/smc-read.c
	src/smc-read.h
	src/apple-smc-reader.cpp
	src/apple-smc-reader.h
	src/main.cpp)

if(APPLE)
	FIND_LIBRARY(IOKIT_LIBRARY IOKit)
	SET(EXTRA_LIBS ${IOKIT_LIBRARY})
endif (APPLE)

target_link_libraries(smc_reader ${EXTRA_LIBS})
