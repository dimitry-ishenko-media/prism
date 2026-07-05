////////////////////////////////////////////////////////////////////////////////
#include <asio.hpp>
#include <iostream>
#include <peel/Gst/Gst.h>
#include "prism/dispatch.hpp"
#include <stdexcept>
#include <string>

////////////////////////////////////////////////////////////////////////////////
using namespace peel;

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
try
{
    Gst::init(&argc, &argv);

    auto pipeline = Gst::Pipeline::create("test-pipeline");

    auto source = Gst::ElementFactory::make("uridecodebin" , "source").ref_sink();
    auto aconv  = Gst::ElementFactory::make("audioconvert" , "convert").ref_sink();
    auto asink  = Gst::ElementFactory::make("autoaudiosink", "sink").ref_sink();
    auto vconv  = Gst::ElementFactory::make("videoconvert" , "vconvert").ref_sink();
    auto vsink  = Gst::ElementFactory::make("autovideosink", "vsink").ref_sink();

    if (!pipeline || !source || !aconv || !asink || !vconv || !vsink)
        throw std::runtime_error{"Not all elements could be created"};

    pipeline->add_many(source, aconv, asink, vconv, vsink);

    if (!aconv->link(asink))
        throw std::runtime_error{"Audio elements could not be linked"};

    if (!vconv->link(vsink))
        throw std::runtime_error{"Video elements could not be linked"};

    source->set_property<String>("uri",
        "https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm"
    );

    source->connect_pad_added([&](Gst::Element* src, Gst::Pad* new_pad)
    {
        auto caps = new_pad->get_current_caps();
        std::string name = caps->get_structure(0)->get_name();

        RefPtr<Gst::Element> sink;
        if (name.starts_with("audio/x-raw")) sink = aconv;
        else if (name.starts_with("video/x-raw")) sink = vconv;

        if (sink)
        {
            auto sink_pad = sink->get_static_pad("sink");
            if (!sink_pad->is_linked()) new_pad->link(sink_pad);
        }
    });

    asio::io_context io;
    auto ex = io.get_executor();

    pri::dispatch dispatch{ex};

    auto bus = pipeline->get_bus();
    bus->set_sync_handler(dispatch);

    dispatch.add_callback(Gst::Message::Type::ERROR_, [&](Gst::Bus*, Gst::Message* msg)
    {
        UniquePtr<GLib::Error> err;
        msg->parse_error(&err, nullptr);

        std::cout << "Error received from " << msg->src->get_name() << ": " << err->message << std::endl;
        io.stop();
    });

    dispatch.add_callback(Gst::Message::Type::EOS, [&](Gst::Bus*, Gst::Message*)
    {
        std::cout << "Reached end of stream" << std::endl;
        io.stop();
    });

    dispatch.add_callback(Gst::Message::Type::STATE_CHANGED, [&](Gst::Bus*, Gst::Message* msg)
    {
        if (msg->src == pipeline)
        {
            Gst::State prev, now, pending;
            msg->parse_state_changed(&prev, &now, &pending);
            std::cout << "Pipeline state changed to " << Gst::state_get_name(now) << std::endl;
        }
    });

    std::cout << "Starting playback" << std::endl;
    if (pipeline->set_state(Gst::State::PLAYING) == Gst::StateChangeReturn::FAILURE)
        throw std::runtime_error{"Unable to start playing"};

    auto work = asio::make_work_guard(ex);
    io.run();

    pipeline->set_state(Gst::State::NULL_);
    return 0;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return 1;
}
