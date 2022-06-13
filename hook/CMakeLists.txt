cmake_minimum_required(VERSION 3.10)

if (CMAKE_TOOLCHAIN_FILE)
  message(STATUS "Using CMAKE_TOOLCHAIN_FILE: " ${CMAKE_TOOLCHAIN_FILE})
else()
  if (NOT "$ENV{CROSS_COMPILE}" STREQUAL "")
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_VERSION 1)
    set(CMAKE_SYSTEM_PROCESSOR "aarch64")
    set(CMAKE_CXX_COMPILER "$ENV{CROSS_COMPILE}g++")
    set(CMAKE_C_COMPILER "$ENV{CROSS_COMPILE}gcc")
  else()
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/toolchains/aarch64-linux-gnu.cmake)
  endif()
endif()

# set the project name
set(HOOK_VERSION_MAJOR 0) # MUST bump this whenever we make ABI-incompatible changes
set(HOOK_VERSION_MINOR 1)
set(HOOK_VERSION_PATCH 0)
set(HOOK_VERSION_STRING ${HOOK_VERSION_MAJOR}.${HOOK_VERSION_MINOR}.${HOOK_VERSION_PATCH})

project(thermal VERSION ${HOOK_VERSION_STRING})

set(CMAKE_BUILD_WITH_INSTALL_RPATH on)

# add the thermal library
add_subdirectory(lib)
add_subdirectory(test)
