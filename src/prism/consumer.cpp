////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "consumer.hpp"
#include <stdexcept>

////////////////////////////////////////////////////////////////////////////////
namespace prism
{

consumer::consumer(std::string id) :
    id_{std::move(id)}
{
    bin_ = Gst::Bin::create(id_.data()).ref_sink();
}

const RefPtr<Gst::Bin>& consumer::get_bin() const
{
    if (!sink_created_)
        throw std::logic_error{"consumer::create_sink() has not been called"};
    return bin_;
}

void consumer::create_sink(Gst::Pad* target)
{
    if (std::exchange(sink_created_, true))
        throw std::logic_error{"consumer::create_sink() called more than once"};
    if (!target) throw std::invalid_argument{"Target pad must not be null"};

    auto pad = Gst::GhostPad::create("sink", target).ref_sink();
    pad->set_active(true);
    bin_->add_pad(pad);
}

}
