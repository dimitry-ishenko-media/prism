////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "channel.hpp"

#include <format>

////////////////////////////////////////////////////////////////////////////////
namespace prism
{

channel::channel(asio::any_io_executor ex, std::string id, prism::video_info video) :
    id_{std::move(id)}, video_{std::move(video)}, dispatch_{ex}
{
    pipeline_ = Gst::Pipeline::create(id_.data()).ref_sink();

    auto create = [](auto type, auto name){
        return Gst::ElementFactory::make(type, name).ref_sink();
    };

    auto src = create("videotestsrc", "src");
    src->set_property<int>("pattern", 2); // black
    src->set_property<bool>("is-live", true);

    auto infil = create("capsfilter", "infil");
    auto caps = Gst::Caps::from_string(std::format(
        "video/x-raw, width=16, height=16, framerate={}/{}",
        video_.fps.num, video_.fps.den
    ).data());
    infil->set_property<Gst::Caps>("caps", caps);

    auto glup = create("glupload", "glup");
    auto glcc = create("glcolorconvert", "glcc");
    auto mixer= create("glvideomixer", "mixer");

    auto exfil= create("capsfilter", "exfil");
    caps = Gst::Caps::from_string(std::format(
        "video/x-raw(memory:GLMemory), format=RGBA, width={}, height={}, framerate={}/{}",
        video_.width, video_.height, video_.fps.num, video_.fps.den
    ).data());
    exfil->set_property<Gst::Caps>("caps", caps);

    auto tee = create("tee", "tee");
    auto queue = create("queue", "queue");

    auto sink = create("fakesink", "sink");
    sink->set_property<bool>("sync", true);

    ////////////////////
    pipeline_->add_many(src, infil, glup, glcc, mixer, exfil, tee, queue, sink);
    src->link_many(infil, glup, glcc);

    auto inpad = mixer->get_request_pad("sink_%u");
    inpad->set_property<int>("width", video_.width);
    inpad->set_property<int>("height", video_.height);
    inpad->set_property<unsigned>("zorder", 0);

    auto outpad = glcc->get_static_pad("src");
    outpad->link(inpad);

    mixer->link_many(exfil, tee, queue, sink);

    ////////////////////
    auto bus = pipeline_->get_bus();
    bus->set_sync_handler(dispatch_);

    dispatch_.add_callback(Gst::Message::Type::ANY, [](Gst::Bus* bus, Gst::Message* msg)
    {
        // TODO
    });
}

channel::~channel()
{
    if (pipeline_) pipeline_->set_state(Gst::State::NULL_);
}

void channel::play()
{
    pipeline_->set_state(Gst::State::PLAYING);
}

void channel::stop()
{
    pipeline_->set_state(Gst::State::READY);
}

}
