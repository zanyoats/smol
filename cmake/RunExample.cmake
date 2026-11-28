cmake_minimum_required(VERSION 3.25)

get_filename_component(smol_root "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
file(RELATIVE_PATH smol_active_path "${smol_root}" "${SMOL_ACTIVE_FILE}")

if(NOT smol_active_path MATCHES "^examples/([A-Za-z0-9_-]+)/main\\.cpp$"
   OR NOT EXISTS "${SMOL_ACTIVE_FILE}")
    message(FATAL_ERROR "Open an examples/<name>/main.cpp file before running this task.")
endif()
set(smol_example "${CMAKE_MATCH_1}")

execute_process(
    COMMAND "${CMAKE_COMMAND}" --preset debug
    WORKING_DIRECTORY "${smol_root}"
    COMMAND_ERROR_IS_FATAL ANY
)
execute_process(
    COMMAND "${CMAKE_COMMAND}" --build --preset debug
        --target "smol_example_${smol_example}" --parallel
    WORKING_DIRECTORY "${smol_root}"
    COMMAND_ERROR_IS_FATAL ANY
)

set(smol_executable "${smol_root}/build/debug/examples/${smol_example}/${smol_example}")
if(WIN32)
    string(APPEND smol_executable ".exe")
endif()
execute_process(
    COMMAND "${smol_executable}"
    WORKING_DIRECTORY "${smol_root}"
    COMMAND_ERROR_IS_FATAL ANY
)
