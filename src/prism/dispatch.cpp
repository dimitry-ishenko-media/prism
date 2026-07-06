////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "dispatch.hpp"

////////////////////////////////////////////////////////////////////////////////
namespace prism
{

Signal<signal, signal::signature> signal::message_received_;

PEEL_CLASS_IMPL(signal, "dispatch_signal", peel::GObject::Object)

void signal::Class::init()
{
    message_received_ = Signal<signal, signature>::create("message-received");
}

}
