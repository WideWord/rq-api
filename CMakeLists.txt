cmake_minimum_required(VERSION 3.2.0)

project(rq)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
SET (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -rpath .")

file(GLOB _SOURCE
		source/*.hpp
		source/*.cpp
		source/**/*.hpp
		source/**/*.cpp
		source/**/**/*.hpp
		source/**/**/*.cpp)

include_directories(/usr/include)
include_directories(/usr/local/include)
include_directories(source/3rdparty)
include_directories(source)
link_directories(/usr/local/lib)

add_executable(rq ${_SOURCE})
