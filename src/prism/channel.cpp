////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "channel.hpp"
#include "consumer.hpp"

#include <format>

////////////////////////////////////////////////////////////////////////////////
namespace prism
{

channel::channel(asio::any_io_executor ex, std::string id, media_info info) :
    id_{std::move(id)}, info_{std::move(info)}, dispatch_{ex}
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
        info_.video.fps.num, info_.video.fps.den
    ).data());
    infil->set_property<Gst::Caps>("caps", caps);

    auto glup = create("glupload", "glup");
    auto glcc = create("glcolorconvert", "glcc");
    mixer_ = create("glvideomixer", "mixer");

    auto exfil= create("capsfilter", "exfil");
    caps = Gst::Caps::from_string(std::format(
        "video/x-raw(memory:GLMemory), format=RGBA, width={}, height={}, framerate={}/{}",
        info_.video.width, info_.video.height,
        info_.video.fps.num, info_.video.fps.den
    ).data());
    exfil->set_property<Gst::Caps>("caps", caps);

    tee_ = create("tee", "tee");
    auto queue = create("queue", "queue");

    auto sink = create("fakesink", "sink");
    sink->set_property<bool>("sync", true);

    ////////////////////
    pipeline_->add_many(src, infil, glup, glcc, mixer_, exfil, tee_, queue, sink);
    src->link_many(infil, glup, glcc);

    auto inpad = mixer_->get_request_pad("sink_%u");
    inpad->set_property<int>("width", info_.video.width);
    inpad->set_property<int>("height", info_.video.height);
    inpad->set_property<unsigned>("zorder", 0);

    auto outpad = glcc->get_static_pad("src");
    outpad->link(inpad);

    mixer_->link_many(exfil, tee_, queue, sink);

    ////////////////////
    auto bus = pipeline_->get_bus();
    bus->set_sync_handler(dispatch_);

    dispatch_.add_callback(Gst::Message::Type::ANY, [](Gst::Bus* bus, Gst::Message* msg)
    {
        // TODO
    });

    pipeline_->set_state(Gst::State::PLAYING);
}

channel::~channel()
{
    if (pipeline_) pipeline_->set_state(Gst::State::NULL_);
}

void channel::add(std::unique_ptr<consumer> con)
{
    auto id = con->id();
    remove(id);

    auto bin = con->get_bin();
    pipeline_->add(bin);

    auto pad = tee_->get_request_pad("src_%u");
    auto sink = bin->get_static_pad("sink");
    if (pad->link(sink) == Gst::Pad::LinkReturn::OK)
    {
        bin->sync_state_with_parent();
        consumers_.emplace(std::move(id), entry{std::move(con), pad});
    }
    else
    {
        pipeline_->remove(bin);
        tee_->release_request_pad(pad);

        // TODO
    }
}

void channel::remove(const std::string& id)
{
    if (auto node = consumers_.extract(id))
    {
        auto entry = std::move(node.mapped());
        entry.pad->add_probe(Gst::Pad::ProbeType::BLOCK_DOWNSTREAM,
            [this, entry = std::move(entry)](Gst::Pad* pad, Gst::Pad::ProbeInfo*)
            {
                auto bin = entry.con->get_bin();
                auto sink = bin->get_static_pad("sink");
                pad->unlink(sink);

                tee_->release_request_pad(pad);

                bin->set_state(Gst::State::NULL_);
                pipeline_->remove(bin);

                return Gst::Pad::ProbeReturn::REMOVE;
            });
    }
}

}
