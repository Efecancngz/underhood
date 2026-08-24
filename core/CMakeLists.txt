add_library(underhood_core STATIC
  src/module_registry.cpp
  src/canvas.cpp
)
target_include_directories(underhood_core PUBLIC include)
target_link_libraries(underhood_core PUBLIC raylib)
