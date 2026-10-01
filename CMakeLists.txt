cmake_minimum_required(VERSION 3.20)

project(kitenge
  VERSION 0.1.0
  DESCRIPTION "A symmetry-based pattern studio"
  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

include(cmake/Dependencies.cmake)

# Shared warning settings for our own targets (not third-party code).
add_library(kitenge_warnings INTERFACE)
if(MSVC)
  target_compile_options(kitenge_warnings INTERFACE /W4)
else()
  target_compile_options(kitenge_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

# Everything except main.cpp goes into a library, so the app and the tests
# build the same code. Sources are picked up from src/ automatically, so
# adding a file to a module never needs an edit here.
file(GLOB_RECURSE KITENGE_SOURCES CONFIGURE_DEPENDS src/*.cpp)
list(REMOVE_ITEM KITENGE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

add_library(kitenge_core STATIC ${KITENGE_SOURCES})
target_include_directories(kitenge_core PUBLIC src)
target_link_libraries(kitenge_core PUBLIC imgui PRIVATE kitenge_warnings)

add_executable(kitenge src/main.cpp)
target_link_libraries(kitenge PRIVATE kitenge_core kitenge_warnings)
