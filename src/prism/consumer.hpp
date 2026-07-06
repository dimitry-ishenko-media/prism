////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <peel/Gst/Gst.h>
#include <string>

////////////////////////////////////////////////////////////////////////////////
namespace prism
{

using namespace peel;

class consumer
{
public:
    explicit consumer(std::string id);
    virtual ~consumer() = default;

    consumer(const consumer&) = delete;
    consumer& operator=(const consumer&) = delete;

    consumer(consumer&&) = delete;
    consumer& operator=(consumer&&) = delete;

    auto& id() const noexcept { return id_; }

    const RefPtr<Gst::Bin>& get_bin() const;

protected:
    std::string id_;
    RefPtr<Gst::Bin> bin_;

    void create_sink(Gst::Pad* target);

private:
    bool sink_created_ = false;
};

}
