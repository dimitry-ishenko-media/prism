////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <peel/Gst/Gst.h>

using namespace peel;

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    Gst::init(&argc, &argv);

    auto pipeline = Gst::parse_launch(
        "playbin uri=https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm", nullptr
    );
    auto bus = pipeline->get_bus();

    pipeline->set_state(Gst::State::PLAYING);
    auto msg = bus->timed_pop_filtered(-1, Gst::Message::Type::ERROR_ | Gst::Message::Type::EOS);
    if (msg->type == Gst::Message::Type::ERROR_) std::cout << "Pipeline error" << std::endl;

    pipeline->set_state(Gst::State::NULL_);
    return 0;
}
