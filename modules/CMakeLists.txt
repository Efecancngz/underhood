if(BUILD_MODULE_UNIQUE_PTR)
  add_subdirectory(unique_ptr)
endif()

if(BUILD_MODULE_SHARED_PTR)
  add_subdirectory(shared_ptr)
endif()

if(BUILD_MODULE_MOVE_SEMANTICS)
  add_subdirectory(move_semantics)
endif()
