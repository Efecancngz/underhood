if(BUILD_MODULE_UNIQUE_PTR)
  add_subdirectory(unique_ptr)
endif()

if(BUILD_MODULE_SHARED_PTR)
  add_subdirectory(shared_ptr)
endif()

if(BUILD_MODULE_MOVE_SEMANTICS)
  add_subdirectory(move_semantics)
endif()

if(BUILD_MODULE_STACK)
  add_subdirectory(stack)
endif()

if(BUILD_MODULE_QUEUE)
  add_subdirectory(queue)
endif()

if(BUILD_MODULE_SINGLY_LINKED_LIST)
  add_subdirectory(singly_linked_list)
endif()

if(BUILD_MODULE_CIRCULAR_LINKED_LIST)
  add_subdirectory(circular_linked_list)
endif()
