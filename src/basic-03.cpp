////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <peel/Gst/Gst.h>
#include "prism/time.hpp"
#include <stdexcept>
#include <string>

using namespace peel;
using namespace pri;

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
try
{
    Gst::init(&argc, &argv);

    auto pipeline = Gst::Pipeline::create("test-pipeline");

    auto source   = Gst::ElementFactory::make("uridecodebin" , "source").ref_sink();
    auto convert  = Gst::ElementFactory::make("audioconvert" , "convert").ref_sink();
    auto resample = Gst::ElementFactory::make("audioresample", "resample").ref_sink();
    auto sink     = Gst::ElementFactory::make("autoaudiosink", "sink").ref_sink();
    auto vconvert = Gst::ElementFactory::make("videoconvert" , "vconvert").ref_sink();
    auto vsink    = Gst::ElementFactory::make("autovideosink", "vsink").ref_sink();

    if (!pipeline || !source || !convert || !resample || !sink || !vconvert || !vsink)
        throw std::runtime_error{"Not all elements could be created"};

    pipeline->add_many(source, convert, resample, sink, vconvert, vsink);

    if (!convert->link_many(resample, sink))
        throw std::runtime_error{"Audio elements could not be linked"};

    if (!vconvert->link(vsink))
        throw std::runtime_error{"Video elements could not be linked"};

    source->set_property<String>("uri",
        "https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm"
    );

    source->connect_pad_added([&](Gst::Element* src, Gst::Pad* new_pad)
    {
        std::cout << "Received new pad " << new_pad->get_name() << " from " << src->get_name() << ":\n";

        auto caps = new_pad->get_current_caps();
        std::string name = caps->get_structure(0)->get_name();

        RefPtr<Gst::Element> sink;
        if (name.starts_with("audio/x-raw")) sink = convert;
        else if (name.starts_with("video/x-raw")) sink = vconvert;

        if (sink)
        {
            auto sink_pad = sink->get_static_pad("sink");
            if (!sink_pad->is_linked())
            {
                if (new_pad->link(sink_pad) != Gst::Pad::LinkReturn::OK)
                    std::cout << "Type is " << name << " but link failed" << std::endl;
                else std::cout << "Link succeeded (type " << name << ")" << std::endl;
            }
            else std::cout << "We are already linked - ignoring" << std::endl;
        }
        else std::cout << "It has type " << name << " which is neither raw audio nor raw video - ignoring" << std::endl;

    });

    if (pipeline->set_state(Gst::State::PLAYING) == Gst::StateChangeReturn::FAILURE)
        throw std::runtime_error{"Unable to start playing"};

    auto bus = pipeline->get_bus();

    UniquePtr<GLib::Error> err;
    String debug_info;

    bool terminate = false;
    do
    {
        auto msg = bus->timed_pop_filtered(time::none,
            Gst::Message::Type::STATE_CHANGED | Gst::Message::Type::ERROR_ | Gst::Message::Type::EOS
        );
        switch (msg->type)
        {
        case Gst::Message::Type::ERROR_:
            msg->parse_error(&err, &debug_info);
            std::cout << "Error received from " << msg->src->get_name() << ": " << err->message << std::endl;
            std::cout << "Debug info: " << (debug_info ? debug_info : "none") << std::endl;
            terminate = true;
            break;

        case Gst::Message::Type::EOS:
            std::cout << "Reached end of stream" << std::endl;
            terminate = true;
            break;

        case Gst::Message::Type::STATE_CHANGED:
            if (msg->src == pipeline)
            {
                Gst::State prev, now, pending;
                msg->parse_state_changed(&prev, &now, &pending);

                auto prev_name = Gst::state_get_name(prev), now_name = Gst::state_get_name(now);
                std::cout << "Pipeline state changed from " << prev_name << " to " << now_name << std::endl;
            }
            break;

        default:
            std::cout << "Unexpected message" << std::endl;
            break;
        }
    }
    while (!terminate);

    pipeline->set_state(Gst::State::NULL_);
    return 0;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return 1;
}
