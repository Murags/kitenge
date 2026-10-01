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

# Sources are picked up from src/ automatically, so adding a file to a
# module never needs an edit here.
file(GLOB_RECURSE KITENGE_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(kitenge ${KITENGE_SOURCES})
target_include_directories(kitenge PRIVATE src)
target_link_libraries(kitenge PRIVATE ${KITENGE_SDL_TARGET})

if(MSVC)
  target_compile_options(kitenge PRIVATE /W4)
else()
  target_compile_options(kitenge PRIVATE -Wall -Wextra -Wpedantic)
endif()
