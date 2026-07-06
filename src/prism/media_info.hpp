////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>

////////////////////////////////////////////////////////////////////////////////
namespace prism
{

struct audio_info
{
    int rate;
    int channels;
    std::string format;
};

struct video_info
{
    int width, height;
    struct { int num, den; } fps;
};

struct media_info
{
    audio_info audio;
    video_info video;
};

}
