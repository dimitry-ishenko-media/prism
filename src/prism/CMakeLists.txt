####################
add_library(prism STATIC
    channel.cpp
)
target_link_libraries(prism PUBLIC peel::peel peel::Gst
    PkgConfig::GSTREAMER PkgConfig::GST_BASE PkgConfig::GST_BAD
)
