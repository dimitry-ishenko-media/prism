####################
add_subdirectory(prism)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE prism)
set_target_properties(main PROPERTIES RUNTIME_OUTPUT_DIRECTORY ".." OUTPUT_NAME "prism")

include(GNUInstallDirs)
install(TARGETS main DESTINATION ${CMAKE_INSTALL_BINDIR})
