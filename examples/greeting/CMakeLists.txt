add_executable(smol_example_greeting main.cpp)
target_link_libraries(smol_example_greeting PRIVATE smol::smol)
set_target_properties(smol_example_greeting PROPERTIES
    OUTPUT_NAME greeting
    CXX_EXTENSIONS OFF
)
