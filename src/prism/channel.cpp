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

channel::channel(std::string id, channel::format fmt) :
    id_{std::move(id)}, fmt_{std::move(fmt)}
{
    using gc = Gst::Caps;
    using ge = Gst::ElementFactory;
    using gp = Gst::Pipeline;

    pipeline_ = gp::create(id_.data()).ref_sink();

    auto src = ge::make("videotestsrc", "src").ref_sink();
    src->set_property<int>("pattern", 2); // black
    src->set_property<bool>("is-live", true);

    auto incaps = ge::make("capsfilter", "incaps").ref_sink();
    auto caps = gc::from_string(std::format(
        "video/x-raw, width=16, height=16, framerate={}/{}",
        fmt_.fps.num, fmt_.fps.den
    ).data());
    incaps->set_property<gc>("caps", caps);

    auto glup = ge::make("glupload", "glup").ref_sink();
    auto glcc = ge::make("glcolorconvert", "glcc").ref_sink();
    auto mixer= ge::make("glvideomixer", "mixer").ref_sink();

    auto outcaps = ge::make("capsfilter", "outcaps").ref_sink();
    caps = gc::from_string(std::format(
        "video/x-raw(memory:GLMemory), format=RGBA, width={}, height={}, framerate={}/{}",
        fmt_.width, fmt_.height, fmt_.fps.num, fmt_.fps.den
    ).data());
    outcaps->set_property<gc>("caps", caps);

    auto tee = ge::make("tee", "tee").ref_sink();
    auto queue = ge::make("queue", "queue").ref_sink();

    auto sink = ge::make("fakesink", "sink").ref_sink();
    sink->set_property<bool>("sync", true);

    ////////////////////
    pipeline_->add_many(src, incaps, glup, glcc, mixer, outcaps, tee, queue, sink);
    src->link_many(incaps, glup, glcc);

    auto inpad = mixer->get_request_pad("sink_%u");
    inpad->set_property<int>("width", fmt_.width);
    inpad->set_property<int>("height", fmt_.height);
    inpad->set_property<unsigned>("zorder", 0);

    auto outpad = glcc->get_static_pad("src");
    outpad->link(inpad);

    mixer->link_many(outcaps, tee, queue, sink);
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
