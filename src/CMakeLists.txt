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
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Activate very verbose makefiles.
set(CMAKE_VERBOSE_MAKEFILE OFF)
# Allow else() and endif() constructs (with empty parentheses)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

SET(CMAKE_BUILD_TYPE Debug CACHE STRING "default to debug" FORCE)

# Setup the cmake directory containing numerous cmake scripts and macros.
set(CMAKE_MODULE_PATH "${liquid_SOURCE_DIR}/cmake")

# Search for the different needed libs. #
#########################################
FIND_PACKAGE(FLEX)
FIND_PACKAGE(BISON)

# We incorporate the CMake features provided by LLVM:
if( $ENV{LLVM_ROOT} )
  set(LLVM_ROOT $ENV{LLVM_ROOT})
endif()
message(STATUS "LLVM root is ${LLVM_ROOT}")
find_package(LLVM PATHS "${LLVM_ROOT}/lib/cmake/llvm" "$ENV{ProgramFiles}/lib/cmake/llvm" "${LLVM_ROOT}/lib")

if( LLVM_FOUND )
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${LLVM_DIR}")
    message(STATUS "Found LLVM in ${LLVM_DIR}")
else()
    message(FATAL_ERROR "LLVM installation not found. Set LLVM_ROOT to the installation root path.")
endif()

include(LLVMConfig)
message(STATUS "Found LLVM Package Version:${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM Built type : ${LLVM_BUILD_TYPE}")
string(REGEX MATCH "[0-9]+\\.[0-9]+" LLVM_VER ${LLVM_PACKAGE_VERSION} )
if(NOT ${LLVM_VER} VERSION_EQUAL "18.0" )
    message(WARNING "LLVM version ${LLVM_VER} is not tested.")
endif()

if(MSVC)
    if(NOT CMAKE_CL_64)
        message(FATAL_ERROR "Currently only 64bit compilation is supported!")
    endif()
endif()

# Let's suppose we want to build a JIT compiler with support for
# binary code :
llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES mcjit interpreter native ipo core Analysis  Support
  TransformUtils Passes
)

# Put all source files into one variable. #
##########################################
set(SOURCES_COMMON
            main.cpp
            buildins.cpp
            AstNode.cpp
            Array.cpp
            Declaration.cpp
            FunctionDeclaration.cpp
            ClassDeclaration.cpp
            CodeGenContext.cpp
            VisitorSyntaxCheck.cpp
            VisitorPrettyPrint.cpp
            tokens.l
            parser.y
            GetOpt.cpp
            Range.cpp
            BinaryOperator.cpp
            UnaryOperator.cpp
            CompareOperator.cpp
            Return.cpp
            WhileLoop.cpp
            Conditional.cpp
            Assignment.cpp
            MethodCall.cpp
   )

set(HEADER_COMMON
            buildins.h
            AstNode.h
            Array.h
            Declaration.h
            FunctionDeclaration.h
            ClassDeclaration.h
            CodeGenContext.h
            Visitor.h
            VisitorSyntaxCheck.h
            VisitorPrettyPrint.h
            GetOpt.h
            BinaryOperator.h
            UnaryOperator.h
            CompareOperator.h
            Range.h
            Return.h
            WhileLoop.h
            Conditional.h
            Assignment.h
            MethodCall.h
   )

set(VER_MAJ 0)
set(VER_MIN 3)
set(VER_REV 3)

configure_file(config.in ${liquid_SOURCE_DIR}/config.h)


BISON_TARGET(Parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp COMPILE_FLAGS --report=solved COMPILE_FLAGS --feature=fixit)
FLEX_TARGET(Scanner tokens.l ${CMAKE_CURRENT_BINARY_DIR}/tokens.cpp )
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)

add_executable(liq ${SOURCES_COMMON} ${HEADER_COMMON} ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS})

# Compiler-dependent and build-depended flags:
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(liq PRIVATE -Wall)
elseif(MSVC)
#    replace_flags("/MDd" "/MTd")
#    replace_flags("/MD" "/MT")
    target_compile_options(liq PRIVATE /W4 /permissive-)
    target_compile_definitions(liq PRIVATE YY_NO_UNISTD_H)
    target_compile_definitions(liq PRIVATE _SCL_SECURE_NO_WARNINGS)
    target_compile_definitions(liq PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(liq PRIVATE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
    set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/tokens.cpp PROPERTIES COMPILE_DEFINITIONS __STDC_VERSION__=199901L)
endif()

# Debug or release ?
target_compile_definitions(liq PRIVATE $<$<CONFIG:Debug>:_DEBUG>)


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

# Add additional include search directories.
target_include_directories(liq PRIVATE ${liquid_SOURCE_DIR} ${LLVM_INCLUDE_DIRS})
target_include_directories(liq PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_compile_features(liq PRIVATE cxx_std_17)

# Finally, we link the LLVM libraries to our executable:
target_link_libraries(liq ${REQ_LLVM_LIBRARIES})

if(MSVC)
    source_group(Header\ Files FILES ${HEADER_COMMON})
    source_group(Source\ Files FILES ${SOURCE_COMMON})
endif()
