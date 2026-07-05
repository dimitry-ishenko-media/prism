####################
include(GNUInstallDirs)

set(GST_LIBS peel::peel peel::Gst
    PkgConfig::GSTREAMER PkgConfig::GST_BASE PkgConfig::GST_BAD
)

add_executable(basic-01 basic-01.cpp)
target_link_libraries(basic-01 PRIVATE ${GST_LIBS})

add_executable(basic-02 basic-02.cpp)
target_link_libraries(basic-02 PRIVATE ${GST_LIBS})

add_executable(basic-03 basic-03.cpp)
target_link_libraries(basic-03 PRIVATE ${GST_LIBS})

add_executable(basic-04 basic-04.cpp)
target_link_libraries(basic-04 PRIVATE ${GST_LIBS})

add_executable(hello-asio hello-asio.cpp)
target_link_libraries(hello-asio PRIVATE ${GST_LIBS})
