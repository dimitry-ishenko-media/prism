////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include <asio.hpp>
#include <iostream>

int main()
{
    std::cout << "Hello world!" << std::endl;

    asio::io_context ctx;
    auto work = asio::make_work_guard(ctx);

    ctx.run();
    return 0;
}
