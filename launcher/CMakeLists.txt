add_executable(underhood main.cpp)
target_link_libraries(underhood PRIVATE underhood_core rlImGui)
# Absolute path baked in at build time -- fine for a dev/portfolio build
# launched from its own build tree; would need real asset packaging for a
# distributable install (v1 simplification, see CLAUDE.md).
target_compile_definitions(underhood PRIVATE UNDERHOOD_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets")

if(BUILD_MODULE_UNIQUE_PTR)
  target_link_libraries(underhood PRIVATE underhood_module_unique_ptr)
endif()
if(BUILD_MODULE_SHARED_PTR)
  target_link_libraries(underhood PRIVATE underhood_module_shared_ptr)
endif()
if(BUILD_MODULE_MOVE_SEMANTICS)
  target_link_libraries(underhood PRIVATE underhood_module_move_semantics)
endif()
if(BUILD_MODULE_STACK)
  target_link_libraries(underhood PRIVATE underhood_module_stack)
endif()
