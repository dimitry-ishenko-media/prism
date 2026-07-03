////////////////////////////////////////////////////////////////////////////////
#include <format>
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

    auto playbin = Gst::ElementFactory::make("playbin" , "playbin").ref_sink();
    if (!playbin) throw std::runtime_error{"Not all elements could be created"};

    playbin->set_property<String>("uri",
        "https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm"
    );
    if (playbin->set_state(Gst::State::PLAYING) == Gst::StateChangeReturn::FAILURE)
        throw std::runtime_error{"Unable to start playing"};

    auto fmt  = [](pri::time time){ return std::format("{:%X}", time); };

    bool playing = false;
    bool seek_enabled = false;
    bool seek_done = false;
    auto len = time::none;

    auto bus = playbin->get_bus();

    UniquePtr<GLib::Error> err;
    String debug_info;

    bool terminate = false;
    do
    {
        using Type = Gst::Message::Type;
        auto mask = Type::STATE_CHANGED | Type::ERROR_ | Type::EOS | Type::DURATION_CHANGED;

        if (auto msg = bus->timed_pop_filtered(100_ms, mask))
        {
            // process message
            switch (msg->type)
            {
            case Type::ERROR_:
                msg->parse_error(&err, &debug_info);
                std::cout << "Error received from " << msg->src->get_name() << ": " << err->message << std::endl;
                std::cout << "Debug info: " << (debug_info ? debug_info : "none") << std::endl;
                terminate = true;
                break;

            case Type::EOS:
                std::cout << "Reached end of stream" << std::endl;
                terminate = true;
                break;

            case Type::DURATION_CHANGED:
                len = time::none;

            case Type::STATE_CHANGED:
                if (msg->src == playbin)
                {
                    Gst::State prev, now, pending;
                    msg->parse_state_changed(&prev, &now, &pending);

                    auto prev_name = Gst::state_get_name(prev), now_name = Gst::state_get_name(now);
                    std::cout << "Pipeline state changed from " << prev_name << " to " << now_name << std::endl;

                    playing = (now == Gst::State::PLAYING);
                    if (playing)
                    {
                        auto query = Gst::Query::create_seeking(Gst::Format::TIME);
                        if (playbin->query(query))
                        {
                            pri::time start, end;
                            query->parse_seeking(nullptr, &seek_enabled, &start, &end);

                            if (seek_enabled)
                                std::cout << "Seeking is ENABLED from " << fmt(start) << " to " << fmt(end) << std::endl;
                            else std::cout << "Seeking is DISABLED for this stream" << std::endl;

                        }
                        else std::cout << "Seeking query failed" << std::endl;
                    }
                }
                break;

            default:
                std::cout << "Unexpected message" << std::endl;
                break;
            }
        }
        else
        {
            // process UI
            if (playing)
            {
                auto pos = time::none;
                if (!playbin->query_position(Gst::Format::TIME, &pos))
                    std::cout << "Could not query current position" << std::endl;

                if (len == time::none)
                    if (!playbin->query_duration(Gst::Format::TIME, &len))
                        std::cout << "Could not query duration" << std::endl;

                if (pos != time::none && len != time::none)
                {
                    std::cout << "Position: " << fmt(pos) << " / " << fmt(len) << "\r" << std::flush;

                    if (seek_enabled && !seek_done && pos > 10_s)
                    {
                        std::cout << "\nReached 10s, performing seek..." << std::endl;
                        playbin->seek_simple(Gst::Format::TIME,
                            Gst::SeekFlags::FLUSH | Gst::SeekFlags::KEY_UNIT, 30_s
                        );
                        seek_done = true;
                    }
                }
            }
        }
    }
    while (!terminate);

    playbin->set_state(Gst::State::NULL_);
    return 0;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return 1;
}
