####################
include(GNUInstallDirs)

add_executable(basic-01 basic-01.cpp)
target_link_libraries(basic-01 PRIVATE peel::peel peel::Gst
    PkgConfig::GSTREAMER PkgConfig::GST_BASE PkgConfig::GST_BAD
)
