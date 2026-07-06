////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "dispatch.hpp"
#include "video_info.hpp"

#include <asio.hpp>
#include <peel/Gst/Gst.h>
#include <string>

////////////////////////////////////////////////////////////////////////////////
namespace prism
{

using namespace peel;

class channel
{
public:
    channel(asio::any_io_executor ex, std::string id, video_info);
    ~channel();

    channel(const channel&) = delete;
    channel& operator=(const channel&) = delete;

    channel(channel&&) = delete;
    channel& operator=(channel&&) = delete;

    auto& id() const noexcept { return id_; }
    auto& video_info() const noexcept { return video_; }

    void play();
    void stop();

private:
    std::string id_;
    prism::video_info video_;

    RefPtr<Gst::Pipeline> pipeline_;
    RefPtr<Gst::Element> mixer_, tee_;

    dispatch<asio::any_io_executor> dispatch_;
};

}
