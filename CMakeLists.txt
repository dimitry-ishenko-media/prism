####################
cmake_minimum_required(VERSION 3.16) # focal
project(prism VERSION 0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(-DVERSION="${PROJECT_VERSION}")

####################
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED IMPORTED_TARGET gstreamer-1.0)
pkg_check_modules(GST_BASE REQUIRED IMPORTED_TARGET gstreamer-plugins-base-1.0)
pkg_check_modules(GST_BAD REQUIRED IMPORTED_TARGET gstreamer-plugins-bad-1.0)

####################
set(peel_SOURCE_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/third_party/peel")
set(peel_BINARY_DIR  "${CMAKE_CURRENT_BINARY_DIR}/_deps/peel-build")
set(peel_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/_deps/peel-install")

if(NOT EXISTS "${peel_INSTALL_DIR}/bin/peel-gen")
    message(STATUS "Boostrapping peel...")
    execute_process(
        COMMAND meson setup "${peel_BINARY_DIR}" "${peel_SOURCE_DIR}" --prefix=${peel_INSTALL_DIR}
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND meson install -C "${peel_BINARY_DIR}" OUTPUT_QUIET
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

list(APPEND CMAKE_PREFIX_PATH "${peel_INSTALL_DIR}")

# FIX for Debian
if(EXISTS "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/gir-1.0")
    set(ENV{GI_GIR_PATH} "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/gir-1.0")
endif()

####################
find_package(peel REQUIRED)
peel_generate(Gst 1.0 RECURSIVE)

add_subdirectory(src)
