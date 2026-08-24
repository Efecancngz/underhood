add_executable(underhood_tests
  test_smoke.cpp
)
target_link_libraries(underhood_tests PRIVATE Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(underhood_tests)
