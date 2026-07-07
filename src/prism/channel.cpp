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

namespace
{

inline auto create(const char* type, const char* name)
{
    auto e = Gst::ElementFactory::make(type, name);
    return std::move(e).ref_sink();
};

}

channel::channel(asio::any_io_executor ex, std::string id, media_info info) :
    ex_{std::move(ex)}, id_{std::move(id)}, info_{std::move(info)}
{
    pipeline_ = Gst::Pipeline::create(id_.data()).ref_sink();
    setup_video_branch();
    setup_audio_branch();

    ////////////////////
    pipeline_->set_state(Gst::State::PLAYING);
}

channel::~channel()
{
    if (pipeline_) pipeline_->set_state(Gst::State::NULL_);
}

void channel::setup_video_branch()
{
    auto src = create("videotestsrc", "vsrc");
    src->set_property<int>("pattern", 2); // black
    src->set_property<bool>("is-live", true);

    auto infil = create("capsfilter", "vinfil");
    auto caps = Gst::Caps::from_string(std::format(
        "video/x-raw, width=16, height=16, framerate={}/{}",
        info_.video.fps.num, info_.video.fps.den
    ).data());
    infil->set_property<Gst::Caps>("caps", caps);

    auto glup = create("glupload", "vglup");
    auto glcc = create("glcolorconvert", "vglcc");
    vmixer_ = create("glvideomixer", "vmixer");

    auto exfil= create("capsfilter", "vexfil");
    caps = Gst::Caps::from_string(std::format(
        "video/x-raw(memory:GLMemory), format=RGBA, width={}, height={}, framerate={}/{}",
        info_.video.width, info_.video.height,
        info_.video.fps.num, info_.video.fps.den
    ).data());
    exfil->set_property<Gst::Caps>("caps", caps);

    vtee_ = create("tee", "vtee");
    auto queue = create("queue", "vqueue");

    auto sink = create("fakesink", "vsink");
    sink->set_property<bool>("sync", false);
    sink->set_property<bool>("async", false);

    ////////////////////
    pipeline_->add_many(src, infil, glup, glcc, vmixer_, exfil, vtee_, queue, sink);
    src->link_many(infil, glup, glcc);

    auto inpad = vmixer_->get_request_pad("sink_%u");
    inpad->set_property<int>("width", info_.video.width);
    inpad->set_property<int>("height", info_.video.height);
    inpad->set_property<unsigned>("zorder", 0);

    auto outpad = glcc->get_static_pad("src");
    outpad->link(inpad);

    vmixer_->link_many(exfil, vtee_, queue, sink);
}

void channel::setup_audio_branch()
{
    auto src = create("audiotestsrc", "asrc");
    src->set_property<int>("wave", 4); // silence
    src->set_property<bool>("is-live", true);

    auto infil = create("capsfilter", "ainfil");
    auto caps = Gst::Caps::from_string(std::format(
        "audio/x-raw, format={}, rate={}, channels={}",
        info_.audio.format, info_.audio.rate, info_.audio.channels
    ).data());
    infil->set_property<Gst::Caps>("caps", caps);

    amixer_ = create("audiomixer", "amixer");

    auto exfil= create("capsfilter", "aexfil");
    exfil->set_property<Gst::Caps>("caps", caps);

    atee_ = create("tee", "atee");
    auto queue = create("queue", "aqueue");

    auto sink = create("fakesink", "asink");
    sink->set_property<bool>("sync", false);
    sink->set_property<bool>("async", false);

    ////////////////////
    pipeline_->add_many(src, infil, amixer_, exfil, atee_, queue, sink);
    src->link_many(infil, amixer_, exfil, atee_, queue, sink);
}

void channel::add(std::unique_ptr<consumer> con)
{
    auto id = con->id();
    remove(id);

    const auto& bin = con->get_bin();
    pipeline_->add(bin);

    const auto& vpad = con->get_vpad();
    const auto& apad = con->get_apad();
    RefPtr<Gst::Pad> vtee_pad, atee_pad;

    bool success = true;
    if (vpad)
    {
        vtee_pad = vtee_->get_request_pad("src_%u");
        success = (vtee_pad->link(vpad) == Gst::Pad::LinkReturn::OK);
    }
    if (success && apad)
    {
        atee_pad = atee_->get_request_pad("src_%u");
        success = (atee_pad->link(apad) == Gst::Pad::LinkReturn::OK);
    }
    if (success)
    {
        bin->sync_state_with_parent();
        consumers_.emplace(std::move(id), entry{ std::move(con), vtee_pad, atee_pad });
    }
    else
    {
        if (vtee_pad)
        {
            if (vtee_pad->is_linked()) vtee_pad->unlink(vpad);
            vtee_->release_request_pad(vtee_pad);
        }
        if (atee_pad)
        {
            if (atee_pad->is_linked()) atee_pad->unlink(apad);
            atee_->release_request_pad(atee_pad);
        }
        pipeline_->remove(bin);

        // TODO
    }
}

void channel::remove(const std::string& id)
{
    if (auto node = consumers_.extract(id))
    {
        auto entry = std::move(node.mapped());
        auto bin = entry.con->get_bin();

        std::shared_ptr<void> async_remove{nullptr, [this, bin](void*){
            asio::post(ex_, [this, bin]{ pipeline_->remove(bin); bin->set_state(Gst::State::NULL_); });
        }};

        if (entry.vtee_pad && entry.vtee_pad->is_linked())
            entry.vtee_pad->add_probe(Gst::Pad::ProbeType::IDLE,
                [this, vpad = entry.con->get_vpad(), async_remove](Gst::Pad* vtee_pad, Gst::Pad::ProbeInfo*)
                {
                    if (vtee_pad->is_linked()) vtee_pad->unlink(vpad);
                    vtee_->release_request_pad(vtee_pad);
                    return Gst::Pad::ProbeReturn::REMOVE;
                });

        if (entry.atee_pad && entry.atee_pad->is_linked())
            entry.atee_pad->add_probe(Gst::Pad::ProbeType::IDLE,
                [this, apad = entry.con->get_apad(), async_remove](Gst::Pad* atee_pad, Gst::Pad::ProbeInfo*)
                {
                    if (atee_pad->is_linked()) atee_pad->unlink(apad);
                    atee_->release_request_pad(atee_pad);
                    return Gst::Pad::ProbeReturn::REMOVE;
                });
    }
}

}
