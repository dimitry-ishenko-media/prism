////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include <peel/class.h>
#include <peel/Gst/Gst.h>

////////////////////////////////////////////////////////////////////////////////
namespace pri
{

using namespace peel;

class signal : public peel::GObject::Object
{
    PEEL_SIMPLE_CLASS(signal, Object)

    using signature = void(Gst::Bus*, Gst::Message*);
    inline static Signal<signal, signature> message_received_;

public:
    static auto create() { return peel::GObject::Object::create<signal>(); }

    template<typename Cb>
    auto connect(Cb&& cb, Gst::Message::Type mask)
    {
        return message_received_.connect(this, GLib::Quark{},
            [cb = std::forward<Cb>(cb), mask](signal*, Gst::Bus* bus, Gst::Message* msg) mutable
            { if (!!(msg->type & mask)) cb(bus, msg); }
        );
    }
    void emit(Gst::Bus* bus, Gst::Message* msg) { message_received_.emit(this, bus, msg); }
};

inline PEEL_CLASS_IMPL(signal, "dispatch_signal", peel::GObject::Object)
inline void signal::Class::init()
{ message_received_ = Signal<signal, signature>::create("message-received"); }

////////////////////////////////////////////////////////////////////////////////
template<typename Executor>
class dispatch
{
    Executor ex_;
    RefPtr<signal> signal_;

public:
    explicit dispatch(Executor ex) :
        ex_{ std::move(ex) }, signal_{ signal::create() }
    { }

    template<typename Cb>
    auto add_callback(Gst::Message::Type mask, Cb&& cb) { return signal_->connect(std::forward<Cb>(cb), mask); }

    auto operator()(Gst::Bus* bus, Gst::Message* msg) const
    {
        post(ex_, [sig = signal_, bus = RefPtr<Gst::Bus>{bus}, msg = RefPtr<Gst::Message>{msg}]
            { sig->emit(bus, msg); }
        );
        return Gst::Bus::SyncReply::DROP;
    }
};

}
