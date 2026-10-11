FetchContent_Declare(doctest
  URL https://github.com/doctest/doctest/archive/refs/tags/v2.5.3.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
FetchContent_MakeAvailable(doctest)

# Test files are picked up automatically: add tests/test_<module>.cpp.
file(GLOB KITENGE_TEST_SOURCES CONFIGURE_DEPENDS test_*.cpp)

add_executable(kitenge_tests main.cpp ${KITENGE_TEST_SOURCES})
target_link_libraries(kitenge_tests PRIVATE kitenge_core doctest::doctest kitenge_warnings)

add_test(NAME kitenge_tests COMMAND kitenge_tests)
