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

screen::screen(std::string id) : consumer{std::move(id)}
{
    auto create = [](auto type, auto name) {
        return Gst::ElementFactory::make(type, name).ref_sink();
    };

    auto queue = create("queue", "queue");
    queue->set_property<unsigned>("max-size-bytes", 0);
    queue->set_property<unsigned>("max-size-time", 0);
    queue->set_property<unsigned>("max-size-buffers", 8);
    queue->set_property<int>("leaky", 2); // leaky

    auto glcc = create("glcolorconvert", "glcc");
    auto sink = create("glimagesink", "sink");
    sink->set_property<bool>("sync", false);
    sink->set_property<bool>("qos", false);

    bin_->add_many(queue, glcc, sink);
    queue->link_many(glcc, sink);

    auto pad = queue->get_static_pad("sink");
    create_sink(pad);
}

}
