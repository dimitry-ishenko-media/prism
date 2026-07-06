////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include <asio.hpp>
#include <iostream>
#include <peel/Gst/Gst.h>

using namespace peel;

int main(int argc, char* argv[])
{
    std::cout << "Hello world!" << std::endl;

    Gst::init(&argc, &argv);
    asio::io_context ctx;

    auto work = asio::make_work_guard(ctx);
    ctx.run();

    return 0;
}
