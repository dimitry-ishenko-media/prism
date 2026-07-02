####################
include(GNUInstallDirs)

set(GST_LIBS peel::peel peel::Gst
    PkgConfig::GSTREAMER PkgConfig::GST_BASE PkgConfig::GST_BAD
)

add_executable(basic-01 basic-01.cpp)
target_link_libraries(basic-01 PRIVATE ${GST_LIBS})
