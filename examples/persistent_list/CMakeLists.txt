add_executable(smol_example_persistent_list main.cpp)
target_link_libraries(smol_example_persistent_list PRIVATE smol::smol)
set_target_properties(smol_example_persistent_list PROPERTIES
    OUTPUT_NAME persistent_list
    CXX_EXTENSIONS OFF
)
