add_executable(smol_example_forward_list main.cpp)
target_link_libraries(smol_example_forward_list PRIVATE smol::smol)
set_target_properties(smol_example_forward_list PROPERTIES
    OUTPUT_NAME forward_list
    CXX_EXTENSIONS OFF
)
