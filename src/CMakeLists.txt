####################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ASIO REQUIRED IMPORTED_TARGET asio)

add_subdirectory(prism)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE PkgConfig::ASIO prism)
set_target_properties(main PROPERTIES RUNTIME_OUTPUT_DIRECTORY ".." OUTPUT_NAME "prism")

include(GNUInstallDirs)
install(TARGETS main DESTINATION ${CMAKE_INSTALL_BINDIR})
