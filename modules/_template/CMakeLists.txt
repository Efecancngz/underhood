# Not built by default -- this folder is a copy-paste starting point.
# After copying to modules/<your_topic>/, add:
#   add_library(underhood_module_<your_topic> OBJECT <your_topic>_module.cpp)
#   target_include_directories(underhood_module_<your_topic> PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#   target_link_libraries(underhood_module_<your_topic> PUBLIC underhood_core)
# to modules/CMakeLists.txt, guarded by a BUILD_MODULE_<YOUR_TOPIC> option
# declared in the root CMakeLists.txt, then link it into launcher/CMakeLists.txt.
