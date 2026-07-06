////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "consumer.hpp"

#include <peel/Gst/Gst.h>
#include <string>

////////////////////////////////////////////////////////////////////////////////
namespace prism
{

using namespace peel;

class screen : public consumer
{
public:
    explicit screen(std::string id);
};

}
