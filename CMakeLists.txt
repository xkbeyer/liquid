
cmake_minimum_required(VERSION 3.1)

project(liquid)

message(STATUS "==================")
message(STATUS "CONFIGURING liquid")
message(STATUS "==================")
message(STATUS "               ")


# Setup the make environement. #
################################

SET(CMAKE_BUILD_TYPE Debug CACHE STRING "default to debug" FORCE)

# Setup the cmake directory containing numerous cmake scripts and macros.
set(CMAKE_MODULE_PATH "${liquid_SOURCE_DIR}/cmake")

# Allow else() and endif() constructs (with empty paranthesis)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Activate very verbose makefiles.
set(CMAKE_VERBOSE_MAKEFILE ON)

FIND_PACKAGE(FLEX)
FIND_PACKAGE(BISON)

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${liquid_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build the mylang. Please go into the \"build\" directory and type \"ccmake ..\" there.\nThank you.")
endif()

# Put all sourcefiles into one variable. #
##########################################
set(SOURCES_COMMON
            main.cpp
            buildins.cpp
            AstNode.cpp
            Declaration.cpp
            FunctionDeclaration.cpp
            ClassDeclaration.cpp
            CodeGenContext.cpp
            VisitorSyntaxCheck.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/tokens.cpp
            tokens.l
            parser.y
   )

set(HEADER_COMMON
            buildins.h
            AstNode.h
            FunctionDeclaration.h
            ClassDeclaration.h
            CodeGenContext.h
            VisitorSyntaxCheck.h
   )

if(MSVC)
    source_group(Header\ Files FILES ${HEADER_COMMON})
    source_group(Source\ Files FILES ${SOURCE_COMMON})
endif()
# Check for various language-stuff and compiler versions #
##########################################################

# Compiler-dependent and build-dependend flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    string(STRIP "${GCC_VERSION}" GCC_VERSION)
    message(STATUS "GCC Version ${GCC_VERSION} detected")
    message(STATUS "GCC is ${CMAKE_CXX_COMPILER} detected")

    # GCC below 4.6 does not have the C++0x features we use.
    if(${GCC_VERSION} VERSION_LESS "4.8")
        message(FATAL_ERROR "You need at least GCC version 4.8 in order to compile mylang")
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wno-non-virtual-dtor -O0 -pg -fstack-protector-all -Winvalid-pch")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s -O2 -Winvalid-pch")
elseif(MSVC)
    add_definitions(-DYY_NO_UNISTD_H)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    #enables C++ Exception
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc ")
endif()

# Debug or release ?
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for debug")
    if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
        add_definitions(-D_DEBUG)
    endif()
endif()


# Search for the different needed libs. #
#########################################

# We incorporate the CMake features provided by LLVM:

if( CMAKE_SYSTEM_NAME MATCHES Linux )
    set(LLVM_ROOT "/usr/llvm" CACHE PATH "Root of LLVM installation.")
elseif( CMAKE_SYSTEM_NAME MATCHES Windows )
    set(LLVM_ROOT "$ENV{ProgramFiles}/llvm" CACHE PATH "Root of LLVM installation.")
else()
    set(LLVM_ROOT "" CACHE PATH "Root of LLVM install.")
endif()
if( NOT EXISTS ${LLVM_ROOT} )
    message(FATAL_ERROR "Invalid setup for LLVM installation in ${LLVM_ROOT}. Set the LLVM_ROOT first and then re-run.")
endif()
# A bit of a sanity check:
if( NOT EXISTS ${LLVM_ROOT}/include/llvm )
    message(FATAL_ERROR "LLVM_ROOT (${LLVM_ROOT}) is not a valid LLVM install")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${LLVM_ROOT}/share/llvm/cmake")
include(LLVMConfig)
message(STATUS "Found LLVM Package Version:${LLVM_PACKAGE_VERSION}")

string(REGEX MATCH "[0-9]+\\.[0-9]+" LLVM_VER ${LLVM_PACKAGE_VERSION} )
set(UseLLVM35 0)
set(UseLLVM36 0)
set(UseMCJIT 0)
set(UseLLVM37 0)
if(${LLVM_VER} VERSION_EQUAL "3.5" )
    set(UseLLVM35 1)
endif()
if(${LLVM_VER} VERSION_EQUAL "3.6" )
    set(UseLLVM36 1)
    set(UseMCJIT 1)
endif()
if(${LLVM_VER} VERSION_EQUAL "3.7" )
    set(UseLLVM37 1)
    set(UseMCJIT 1)
endif()
if(${LLVM_VER} VERSION_LESS "3.5" )
    message(FATAL_ERROR "LLVM version before 3.5 are not supported. Abort!")
endif()

if(MSVC)
    if(CMAKE_CL_64 AND NOT UseLLVM37)
        message(FATAL_ERROR "Currently the LLVM ${LLVM_VER} 64bit doesn't work well under Win64 therefore it isn't supported yet!")
    endif()
endif()


configure_file(config.in ${liquid_SOURCE_DIR}/config.h)

# Now set the header and library paths:
include_directories( ${LLVM_ROOT}/include )
link_directories( ${LLVM_ROOT}/lib )

# Let's suppose we want to build a JIT compiler with support for
# binary code :
if( UseMCJIT )
    llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES mcjit interpreter native)
else()
    llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES jit interpreter native)
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(${LLVM_DEFINITIONS})
endif()


# Add additional include search directories.
include_directories(${liquid_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Add LLVM-FLAGS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM-FLAGS}")
message(STATUS "All CXX Flags: ${CMAKE_CXX_FLAGS}")

# The compiling process. #
##########################

BISON_TARGET(Parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
FLEX_TARGET(Scanner tokens.l ${CMAKE_CURRENT_BINARY_DIR}/tokens.cpp )
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(liq ${SOURCES_COMMON} ${HEADER_COMMON} ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS})
set_property(TARGET liq PROPERTY CXX_STANDARD 14)
set_property(TARGET liq PROPERTY CXX_STANDARD_REQUIRED ON)

# Finally, we link the LLVM libraries to our executable:
target_link_libraries(liq ${REQ_LLVM_LIBRARIES})

