cmake_minimum_required(VERSION 3.12)

project(liquid)

# Setup the make environment. #
################################
# Activate very verbose makefiles.
set(CMAKE_VERBOSE_MAKEFILE OFF)
# Allow else() and endif() constructs (with empty parentheses)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) #To use with clang-tidy, not supported w/ MSVC generator.

# Setup the cmake directory containing numerous cmake scripts and macros.
set(CMAKE_MODULE_PATH "${liquid_SOURCE_DIR}/cmake")

add_subdirectory(src)

# Set the visual studio start up project.
if(MSVC)
   set_property(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} PROPERTY VS_STARTUP_PROJECT liq)
endif()