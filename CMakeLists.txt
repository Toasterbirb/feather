cmake_minimum_required(VERSION 3.12)

project(Feather)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(TESTS "Enable tests in the build" OFF)

include_directories(include/ vendor/)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/seed)

if (NOT TESTS)
	add_definitions(-DDOCTEST_CONFIG_DISABLE)
else()
	add_definitions(-DTEST)
endif()

add_executable(feather
	src/main.cpp
	src/parser.cpp)

target_link_libraries(feather seed)
target_compile_options(feather PRIVATE -std=c++17)
