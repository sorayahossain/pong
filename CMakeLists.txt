cmake_minimum_required(VERSION 3.20)
project(Pong LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# vendor/ only ships prebuilt raylib binaries for Windows, so on other
# platforms we look for a system install first and fall back to building
# raylib from source (pinned to the same version vendored for Windows).
find_package(raylib 5.5 QUIET)
if(NOT raylib_FOUND)
	include(FetchContent)
	set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
	FetchContent_Declare(
		raylib
		GIT_REPOSITORY https://github.com/raysan5/raylib.git
		GIT_TAG 5.5
	)
	FetchContent_MakeAvailable(raylib)
endif()

add_executable(Pong Pong/src/main.cpp)
target_link_libraries(Pong PRIVATE raylib)
