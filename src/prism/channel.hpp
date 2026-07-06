////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "dispatch.hpp"

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
    struct format
    {
        int width;
        int height;
        struct { int num, den; } fps;
    };

    channel(asio::any_io_executor ex, std::string id, format);
    ~channel();

    channel(const channel&) = delete;
    channel& operator=(const channel&) = delete;

    channel(channel&&) = delete;
    channel& operator=(channel&&) = delete;

    void play();
    void stop();

private:
    std::string id_;
    format fmt_;

    RefPtr<Gst::Pipeline> pipeline_;
    dispatch<asio::any_io_executor> dispatch_;
};

}
