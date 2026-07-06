####################
add_library(prism STATIC
    channel.cpp
    consumer.cpp
    dispatch.cpp

    consumer/screen.cpp
)
target_include_directories(prism PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(prism PUBLIC peel::peel peel::Gst
    PkgConfig::GSTREAMER PkgConfig::GST_BASE PkgConfig::GST_BAD
)
