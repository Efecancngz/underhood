add_executable(underhood_tests
  test_smoke.cpp
  test_module_registry.cpp
  test_unique_ptr_module.cpp
  test_shared_ptr_module.cpp
)
target_link_libraries(underhood_tests PRIVATE
  underhood_core
  underhood_module_unique_ptr
  underhood_module_shared_ptr
  Catch2::Catch2WithMain
)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(underhood_tests)
