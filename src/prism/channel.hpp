////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "dispatch.hpp"
#include "media_info.hpp"

#include <asio.hpp>
#include <memory>
#include <peel/Gst/Gst.h>
#include <string>
#include <unordered_map>

////////////////////////////////////////////////////////////////////////////////
namespace prism
{

using namespace peel;
class consumer;

class channel
{
public:
    channel(asio::any_io_executor ex, std::string id, media_info);
    ~channel();

    channel(const channel&) = delete;
    channel& operator=(const channel&) = delete;

    channel(channel&&) = delete;
    channel& operator=(channel&&) = delete;

    auto& id() const noexcept { return id_; }
    auto& info() const noexcept { return info_; }

    void add(std::unique_ptr<consumer>);
    void remove(const std::string& id);

private:
    std::string id_;
    media_info info_;

    RefPtr<Gst::Pipeline> pipeline_;
    RefPtr<Gst::Element> mixer_, tee_;

    dispatch<asio::any_io_executor> dispatch_;

    struct entry
    {
        std::unique_ptr<consumer> con;
        RefPtr<Gst::Pad> pad;
    };
    std::unordered_map<std::string, entry> consumers_;
};

}
