cmake_minimum_required (VERSION 2.8.12)

project (statsdcc CXX)

include(CheckIncludeFileCXX)
include (CheckCXXCompilerFlag)

set (PROJECT_VERSION_MAJOR 0)
set (PROJECT_VERSION_MINOR 14)
set (PROJECT_VERSION_PATCH 0)
set (PROJECT_VERSION
    "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
execute_process (
  COMMAND git log -1 --format=%h
  OUTPUT_VARIABLE BUILD
  OUTPUT_STRIP_TRAILING_WHITESPACE)

if (NOT debug)
  set (debug "false")
endif()


configure_file (include/statsdcc/version.h.in
                include/statsdcc/version.h)

# set compiler flags
check_cxx_compiler_flag ('-std=c++11' COMPILER_SUPPORTS_CXX11)
check_cxx_compiler_flag ('-std=c++0x' COMPILER_SUPPORTS_CXX0X)

if(COMPILER_SUPPORTS_CXX11)
  add_compile_options (-std=c++11)
  set (CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
  add_compile_options (-std=c++0x)
  set (CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -std=c++0x")
else()
  message(STATUS
    "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler."
  )
endif()

add_compile_options (-Wall -Wextra)

add_definitions (-D_GLIBCXX_USE_NANOSLEEP)
add_definitions (-D_GLIBCXX_USE_SCHED_YIELD)

if (debug)
  add_compile_options(-g)
else(debug)
  add_compile_options(-O3)
endif()

include_directories (
   include
   build/include
   vendor/gtest-1.7.0/include
)

# check if atomic.h exists
check_include_file_cxx (atomic HAVE_ATOMIC)
check_include_file_cxx (cstdatomic HAVE_CSTDATOMIC)

if (HAVE_ATOMIC)
  add_definitions(-DHAVE_ATOMIC)
endif()

if (HAVE_CSTDATOMIC)
  add_definitions(-DHAVE_CSTDATOMIC)
endif()

# find libraries
function (lib_not_found library_name)
  message (FATAL_ERROR
    "Unable to find ${library_name}. "
    "Try setting the path to ${library_name} in environment variable CMAKE_LIBRARY_PATH."
  )
endfunction(lib_not_found)

find_library (CRYPTO NAMES libcrypto.a crypto)
if (NOT CRYPTO)
  lib_not_found("libcrypto.a")
endif()

find_library (JSONCPP NAMES libjsoncpp.a jsoncpp)
if (NOT JSONCPP)
  lib_not_found("libjsoncpp.a")
endif()

find_path (JSONCPP_INCLUDE_DIR NAMES json/json.h PATH_SUFFIXES jsoncpp)
if (NOT JSONCPP_INCLUDE_DIR)
  message (FATAL_ERROR "Unable to find headers for JSONCPP.")
endif()
include_directories (${JSONCPP_INCLUDE_DIR})

find_library (BOOST_REGEX NAMES libboost_regex.a boost_regex)
if (NOT BOOST_REGEX)
  lib_not_found("libboost_regex.a")
endif()

find_library (TCMALLOC_MINIMAL NAMES libtcmalloc_minimal.a tcmalloc_minimal)
if (NOT TCMALLOC_MINIMAL)
  lib_not_found("libtcmalloc_minimal.a")
endif()

find_library (MICROHTTPD NAMES libmicrohttpd.so microhttpd)
if (NOT MICROHTTPD)
  lib_not_found("libmicrohttpd.so")
endif()

add_subdirectory (lib)
add_subdirectory (src)

if (test)
  enable_testing()

  add_subdirectory (test)
  add_subdirectory (vendor/gtest-1.7.0)

  add_test (ledger_test test/ledger_test)

  if (RUN_NODE_HASHRING_TESTS)
    add_test (hashring_test test/hashring_test)
  endif()

endif()
