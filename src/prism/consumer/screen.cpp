////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "consumer/screen.hpp"

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

screen::screen(std::string id) : consumer{std::move(id)}
{
    auto queue = create("queue", "vqueue");
    queue->set_property<unsigned>("max-size-bytes", 0);
    queue->set_property<unsigned>("max-size-time", 0);
    queue->set_property<unsigned>("max-size-buffers", 8);
    queue->set_property<int>("leaky", 2); // leaky

    auto glcc = create("glcolorconvert", "vglcc");
    auto sink = create("glimagesink", "vsink");
    sink->set_property<bool>("sync", false);
    sink->set_property<bool>("qos", false);

    bin_->add_many(queue, glcc, sink);
    queue->link_many(glcc, sink);

    auto pad = queue->get_static_pad("sink");
    vpad_ = Gst::GhostPad::create("sink", pad).ref_sink();
    vpad_->set_active(true);
    bin_->add_pad(vpad_);
}

}
