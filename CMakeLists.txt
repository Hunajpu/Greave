# CMake build file for Greave, intended for use with MinGW/GCC on Windows and GCC on Linux. Support for OS X, Visual Studio, clang, etc. is not currently available.
# For MinGW/MSYS builds, be sure to specify -G"MinGW Makefiles" when setting up the build folder.
# Copyright (c) 2020-2021 Raine "Gravecat" Simmons and the Greave contributors. Licensed under the GNU Affero General Public License v3 or any later version.

cmake_minimum_required(VERSION 3.13)
project(greave)

# Automatically default to release builds if a type is not specified. This code borrowed from BearLibTerminal, copyright (c) 2013-2017 Cfyz.
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Build type was not specified, switching to 'Release'.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

# Platform-specific defaults/overrides
if(WIN32)
  set(TARGET_WINDOWS 1)
  add_definitions(-DGREAVE_TARGET_WINDOWS)
else()
  set(TARGET_LINUX 1)
  add_definitions(-DGREAVE_TARGET_LINUX)
endif(WIN32)

# Detect 32-bit or 64-bit build
if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
  set(TARGET_32 1)
else()
  set(TARGET_64 1)
endif()

add_subdirectory(src)
add_subdirectory(binfiles)
