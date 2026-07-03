////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <peel/Gst/Gst.h>
#include "prism/time.hpp"
#include <stdexcept>

using namespace peel;
using namespace pri;

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
try
{
    Gst::init(&argc, &argv);

    auto pipeline = Gst::Pipeline::create("test-pipeline");
    auto source = Gst::ElementFactory::make("videotestsrc", "source").ref_sink();
    auto sink = Gst::ElementFactory::make("autovideosink", "sink").ref_sink();

    if (!pipeline || !source || !sink) throw std::runtime_error{"Not all elements could be created"};
#if 1
    pipeline->add_many(source, sink);
    if (!source->link(sink)) throw std::runtime_error{"Elements could not be linked"};
#else
    auto fail = Gst::ElementFactory::make("identity", "fail").ref_sink();
    fail->set_property<int>("error-after", 300);

    pipeline->add_many(source, fail, sink);
    if (!source->link_many(fail, sink))
        throw std::runtime_error{"Elements could not be linked"};
#endif
    source->set_property<int>("pattern", 0);

    if (pipeline->set_state(Gst::State::PLAYING) == Gst::StateChangeReturn::FAILURE)
        throw std::runtime_error{"Unable to start playing"};

    auto bus = pipeline->get_bus();
    auto msg = bus->timed_pop_filtered(time::none, Gst::Message::Type::ERROR_ | Gst::Message::Type::EOS);
    
    UniquePtr<GLib::Error> err;
    String debug_info;

    switch (msg->type)
    {
    case Gst::Message::Type::ERROR_:
        msg->parse_error(&err, &debug_info);
        std::cout << "Error received from " << msg->src->get_name() << ": " << err->message << std::endl;
        std::cout << "Debug info: " << (debug_info ? debug_info : "none") << std::endl;
        break;

    case Gst::Message::Type::EOS:
        std::cout << "Reached end of stream" << std::endl;
        break;

    default:
        std::cout << "Unexpected message" << std::endl;
        break;
    }

    pipeline->set_state(Gst::State::NULL_);
    return 0;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return 1;
}
