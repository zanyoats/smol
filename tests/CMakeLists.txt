# Build Catch2 and our tests with the same standard so its string_view
# assertion diagnostics are available when linking the tests.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(Catch2
    URL https://codeload.github.com/catchorg/Catch2/tar.gz/refs/tags/v3.16.1
    URL_HASH SHA256=d16bd1b8b2364918bd472cb216394acb93bde6784890df0d91bccfb3f28d777d
    SYSTEM
    FIND_PACKAGE_ARGS 3.16.1 EXACT
)
FetchContent_MakeAvailable(Catch2)

add_executable(smol_tests greeting.cpp persistent_list.cpp)
target_link_libraries(smol_tests PRIVATE smol::smol Catch2::Catch2WithMain)
set_target_properties(smol_tests PROPERTIES CXX_EXTENSIONS OFF)

if(MSVC)
    target_compile_options(smol_tests PRIVATE /W4 /permissive-)
    if(SMOL_WARNINGS_AS_ERRORS)
        target_compile_options(smol_tests PRIVATE /WX)
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang|GNU)$")
    target_compile_options(smol_tests PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wshadow
    )
    if(SMOL_WARNINGS_AS_ERRORS)
        target_compile_options(smol_tests PRIVATE -Werror)
    endif()
endif()

if(SMOL_ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang|GNU)$" AND NOT WIN32)
        target_compile_options(smol_tests PRIVATE
            -fsanitize=address,undefined -fno-omit-frame-pointer
        )
        target_link_options(smol_tests PRIVATE -fsanitize=address,undefined)
    else()
        message(FATAL_ERROR "SMOL_ENABLE_SANITIZERS requires GCC or Clang on a non-Windows platform")
    endif()
endif()

# Catch2 provides this module for both installed and fetched dependencies.
include(Catch)
catch_discover_tests(smol_tests)
