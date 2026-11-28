cmake_minimum_required(VERSION 3.25)
project(smol VERSION 0.1.0 LANGUAGES CXX)

option(SMOL_BUILD_TESTS "Build smol tests" ${PROJECT_IS_TOP_LEVEL})
option(SMOL_BUILD_EXAMPLES "Build smol examples" ${PROJECT_IS_TOP_LEVEL})
option(SMOL_ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)
option(SMOL_WARNINGS_AS_ERRORS "Treat warnings in smol tests as errors" OFF)

include(GNUInstallDirs)

# Templates live in headers. Consumers inherit includes and the language requirement,
# but never our development warnings or sanitizer flags.
add_library(smol INTERFACE)
add_library(smol::smol ALIAS smol)
target_compile_features(smol INTERFACE cxx_std_20)
target_include_directories(smol INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

include(CMakePackageConfigHelpers)
install(TARGETS smol EXPORT smolTargets)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT smolTargets NAMESPACE smol:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/smol)
configure_package_config_file(cmake/smolConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/smolConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/smol
)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/smolConfigVersion.cmake
    VERSION ${PROJECT_VERSION} COMPATIBILITY SameMinorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/smolConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/smolConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/smol
)

if(SMOL_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(SMOL_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
