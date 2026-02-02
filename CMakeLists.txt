cmake_minimum_required(VERSION 3.21)

# Use C++20 or 23 as required by Geode
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Update project name and version to match your mod.json
project(ThePissList VERSION 1.0.0)

# This grabs all .cpp files in src/ and its subfolders.
# Ensure you have deleted IDPackCell.cpp, IDPackLayer.cpp, and LevelBrowserLayer.cpp!
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK!")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# Jasmine-tools is required for the UI elements and search logic used in the fork
CPMAddPackage("gh:hiimjasmine00/jasmine-tools@1.0.1")
target_link_libraries(${PROJECT_NAME} jasmine-tools)

setup_geode_mod(${PROJECT_NAME})