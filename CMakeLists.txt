cmake_minimum_required(VERSION 3.20)
project(underhood CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_MODULE_UNIQUE_PTR "Build the unique_ptr simulation module" ON)
option(BUILD_MODULE_SHARED_PTR "Build the shared_ptr simulation module" ON)
option(BUILD_MODULE_MOVE_SEMANTICS "Build the move_semantics simulation module" ON)
option(BUILD_MODULE_STACK "Build the stack simulation module" ON)
option(BUILD_MODULE_QUEUE "Build the queue simulation module" ON)

include(FetchContent)

FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.6.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(catch2)

FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 5.5
  GIT_SHALLOW TRUE
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

FetchContent_Declare(
  imgui_src
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG docking
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui_src)

add_library(imgui STATIC
  ${imgui_src_SOURCE_DIR}/imgui.cpp
  ${imgui_src_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_src_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_src_SOURCE_DIR}/imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC ${imgui_src_SOURCE_DIR})

FetchContent_Declare(
  rlimgui_src
  GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
  GIT_TAG main
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(rlimgui_src)

add_library(rlImGui STATIC ${rlimgui_src_SOURCE_DIR}/rlImGui.cpp)
target_include_directories(rlImGui PUBLIC ${rlimgui_src_SOURCE_DIR})
target_link_libraries(rlImGui PUBLIC raylib imgui)

add_subdirectory(core)
add_subdirectory(modules)
add_subdirectory(launcher)

enable_testing()
add_subdirectory(tests)
