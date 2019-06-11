cmake_minimum_required(VERSION 3.12)

project(liquid)

message(STATUS "==================")
message(STATUS "CONFIGURING liquid")
message(STATUS "==================")

macro(replace_flags flag newflag)
    FOREACH(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
        IF(${flag_var} MATCHES ${flag})
            STRING(REGEX REPLACE "${flag}" "${newflag}" ${flag_var} "${${flag_var}}")
        ENDIF()
    ENDFOREACH(flag_var)
endmacro()

# Setup the make environment. #
################################
# Activate very verbose makefiles.
set(CMAKE_VERBOSE_MAKEFILE ON)
# Allow else() and endif() constructs (with empty parentheses)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

SET(CMAKE_BUILD_TYPE Debug CACHE STRING "default to debug" FORCE)

# Setup the cmake directory containing numerous cmake scripts and macros.
set(CMAKE_MODULE_PATH "${liquid_SOURCE_DIR}/cmake")

FIND_PACKAGE(FLEX)
FIND_PACKAGE(BISON)

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

BISON_TARGET(Parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
FLEX_TARGET(Scanner tokens.l ${CMAKE_CURRENT_BINARY_DIR}/tokens.cpp )
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)

add_executable(liq ${SOURCES_COMMON} ${HEADER_COMMON} ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS})

# Check for various language-stuff and compiler versions #
##########################################################

# Compiler-dependent and build-depended flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(liq PRIVATE -Wall)
elseif(MSVC)
    replace_flags("/MDd" "/MTd")
    replace_flags("/MD" "/MT")
    target_compile_definitions(liq PRIVATE YY_NO_UNISTD_H)
    target_compile_definitions(liq PRIVATE _SCL_SECURE_NO_WARNINGS)
    target_compile_definitions(liq PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(liq PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
    set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/tokens.cpp PROPERTIES COMPILE_DEFINITIONS __STDC_VERSION__=199901L)
endif()

# Debug or release ?
target_compile_definitions(liq PRIVATE $<$<CONFIG:Debug>:_DEBUG>)

# Search for the different needed libs. #
#########################################

# We incorporate the CMake features provided by LLVM:
if( $ENV{LLVM_ROOT} )
  set(LLVM_ROOT $ENV{LLVM_ROOT})
endif()
find_package(LLVM
  PATHS "${LLVM_ROOT}/lib/cmake/llvm" "$ENV{ProgramFiles}/lib/cmake/llvm")

if( LLVM_FOUND )
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${LLVM_DIR}")
else()
    message(FATAL_ERROR "LLVM installation not found. Set LLVM_ROOT to the installation root path.")
endif()

include(LLVMConfig)
message(STATUS "Found LLVM Package Version:${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM Built type : ${LLVM_BUILD_TYPE}")
string(REGEX MATCH "[0-9]+\\.[0-9]+" LLVM_VER ${LLVM_PACKAGE_VERSION} )
if(${LLVM_VER} VERSION_LESS "6.0" )
    message(FATAL_ERROR "LLVM version before 6.0 are not supported. Abort!")
endif()
if(${LLVM_VER} VERSION_GREATER "8.0" )
    message(WARNING "LLVM version ${LLVM_VER} is not tested.")
endif()

if(MSVC)
    if(NOT CMAKE_CL_64)
        message(FATAL_ERROR "Currently only 64bit compilation is supported!")
    endif()
endif()

# Add the compiler definitions of LLVM
if(MSVC)
    # In order to use it with target_compile_definitions we have to replace the -D
    # with a semicolon otherwise they are not correctly populated to MSVC and the
    # IntelliSense is broken (command line error).
    string(REPLACE "-D" ";" LLVM_DEFINITIONS ${LLVM_DEFINITIONS} )
endif()
target_compile_definitions(liq PRIVATE ${LLVM_DEFINITIONS})
if( "${LLVM_BUILD_TYPE}" MATCHES  "Release|RelWithDebInfo|MinSizeRel")
  target_compile_definitions(liq PRIVATE LLVM_NO_DUMP)
endif()

# Now set the header and library paths:
include_directories( ${LLVM_ROOT}/include )
link_directories( ${LLVM_ROOT}/lib )

# Let's suppose we want to build a JIT compiler with support for
# binary code :
llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES mcjit interpreter native ipo)

if(NOT LLVM_ENABLE_RTTI)
    target_compile_definitions(liq PRIVATE LIQ_NO_RTTI)
    if(MSVC)
        target_compile_options(liq PRIVATE /GR-)
    else()
        target_compile_options(liq PRIVATE -fno-rtti)
    endif()
endif()

if(LLVM_ENABLE_EH AND CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(liq PRIVATE -fexceptions)
endif()

set(VER_MAJ 0)
set(VER_MIN 2)
set(VER_REV 0)

configure_file(config.in ${liquid_SOURCE_DIR}/config.h)

# The compiling process. #
##########################

# Add additional include search directories.
target_include_directories(liq PRIVATE ${liquid_SOURCE_DIR})
target_include_directories(liq PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

set_property(TARGET liq PROPERTY CXX_STANDARD 17)
set_property(TARGET liq PROPERTY CXX_STANDARD_REQUIRED ON)

# Finally, we link the LLVM libraries to our executable:
target_link_libraries(liq ${REQ_LLVM_LIBRARIES})
