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

class channel
{
public:
    struct format
    {
        int width;
        int height;
        struct { int num, den; } fps;
    };

    channel(std::string id, format);
    ~channel();

    channel(const channel&) = delete;
    channel& operator=(const channel&) = delete;

    channel(channel&&) = default;
    channel& operator=(channel&&) = default;

    void play();
    void stop();

private:
    std::string id_;
    format fmt_;

    RefPtr<Gst::Pipeline> pipeline_;
};

}
