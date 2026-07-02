####################
include(GNUInstallDirs)

add_executable(prism main.cpp)
target_link_libraries(prism PRIVATE peel::peel peel::Gst
    PkgConfig::GSTREAMER PkgConfig::GST_BASE PkgConfig::GST_BAD
)

install(TARGETS prism DESTINATION ${CMAKE_INSTALL_BINDIR})
