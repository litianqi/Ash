add_executable(AshEditor editor.cpp)
target_link_libraries(AshEditor PRIVATE Ash)
target_include_directories(AshEditor PRIVATE ${ASH_INCLUDE_DIR})
