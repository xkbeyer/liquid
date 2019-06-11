cmake_minimum_required(VERSION 3.12)

project(liquid)

# Setup the make environment. #
################################
# Activate very verbose makefiles.
set(CMAKE_VERBOSE_MAKEFILE ON)
# Allow else() and endif() constructs (with empty parentheses)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Setup the cmake directory containing numerous cmake scripts and macros.
set(CMAKE_MODULE_PATH "${liquid_SOURCE_DIR}/cmake")

add_subdirectory(src)