cmake_minimum_required(VERSION 3.2)

project(liquid)

message(STATUS "==================")
message(STATUS "CONFIGURING liquid")
message(STATUS "==================")

macro(replace_flags flag newflag)
    FOREACH(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
        message(STATUS "repl ${flag_var} ${flag} ${newflag}")  
        IF(${flag_var} MATCHES ${flag})
            STRING(REGEX REPLACE "${flag}" "${newflag}" ${flag_var} "${${flag_var}}")
        ENDIF()
    ENDFOREACH(flag_var)
endmacro()

# Setup the make environment. #
################################

SET(CMAKE_BUILD_TYPE Debug CACHE STRING "default to debug" FORCE)

# Setup the cmake directory containing numerous cmake scripts and macros.
set(CMAKE_MODULE_PATH "${liquid_SOURCE_DIR}/cmake")

# Allow else() and endif() constructs (with empty parentheses)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

# Activate very verbose makefiles.
set(CMAKE_VERBOSE_MAKEFILE ON)

FIND_PACKAGE(FLEX)
FIND_PACKAGE(BISON)

# Disallow in-source builds. #
##############################
EXECUTE_PROCESS(COMMAND pwd OUTPUT_VARIABLE CURR_DIR)
if("${CURR_DIR}" STREQUAL "${liquid_SOURCE_DIR}\n")
    message(FATAL_ERROR "In-source-builds are not allowed to build the liquid. Please go into the \"build\" directory and type \"ccmake ..\" there.\nThank you.")
endif()

# Put all source files into one variable. #
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
            VisitorPrettyPrint.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/tokens.cpp
            tokens.l
            parser.y
            GetOpt.cpp
            Array.cpp
            Range.cpp
   )

set(HEADER_COMMON
            buildins.h
            AstNode.h
            FunctionDeclaration.h
            ClassDeclaration.h
            CodeGenContext.h
            Visitor.h
            VisitorSyntaxCheck.h
            VisitorPrettyPrint.h
            GetOpt.h
   )

if(MSVC)
    source_group(Header\ Files FILES ${HEADER_COMMON})
    source_group(Source\ Files FILES ${SOURCE_COMMON})
endif()

# Check for various language-stuff and compiler versions #
##########################################################

# Compiler-dependent and build-depended flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    add_compile_options(-Wall)
elseif(MSVC)
    add_definitions(-DYY_NO_UNISTD_H)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D__STDC_VERSION__=199901L) #flex/bison to avoid redefinition of int types.
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
if(${LLVM_VER} VERSION_LESS "3.8" )
    message(FATAL_ERROR "LLVM version before 3.8 are not supported. Abort!")
endif()

if(MSVC)
    if(NOT CMAKE_CL_64)
        message(FATAL_ERROR "Currently only 64bit compilation is supported!")
    endif()
endif()

# Add the compiler definitions of LLVM
add_definitions(${LLVM_DEFINITIONS})


# Now set the header and library paths:
include_directories( ${LLVM_ROOT}/include )
link_directories( ${LLVM_ROOT}/lib )

# Let's suppose we want to build a JIT compiler with support for
# binary code :
llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES mcjit interpreter native ipo)


if(NOT LLVM_ENABLE_RTTI)
    add_definitions(-DLIQ_NO_RTTI)
    if(MSVC)
        add_compile_options(/GR-)
    else()
        add_compile_options(-fno-rtti)
    endif()
endif()

#enables C++ Exception
if(LLVM_ENABLE_EH)
    if(MSVC)
        add_compile_options(/EHsc)
    else()
        add_compile_options(-fexceptions)
    endif()
else()
    if(MSVC)
        # Don't remove C++ exception for MSVC since they are needed.
        add_compile_options(/EHsc)
    endif()
endif()

set(VER_MAJ 0)
set(VER_MIN 2)
set(VER_REV 0)

configure_file(config.in ${liquid_SOURCE_DIR}/config.h)

# The compiling process. #
##########################

# Add additional include search directories.
include_directories(${liquid_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

BISON_TARGET(Parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
FLEX_TARGET(Scanner tokens.l ${CMAKE_CURRENT_BINARY_DIR}/tokens.cpp )
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(liq ${SOURCES_COMMON} ${HEADER_COMMON} ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS})
set_property(TARGET liq PROPERTY CXX_STANDARD 14)
set_property(TARGET liq PROPERTY CXX_STANDARD_REQUIRED ON)

# Finally, we link the LLVM libraries to our executable:
target_link_libraries(liq ${REQ_LLVM_LIBRARIES})
