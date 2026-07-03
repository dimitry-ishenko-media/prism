////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <chrono>
#include <concepts>
#include <format>
#include <peel/Gst/Clock.h>
#include <type_traits>

////////////////////////////////////////////////////////////////////////////////
namespace pri
{

class time
{
public:
    using rep = gint64;
    using period = std::nano;
    using duration = std::chrono::duration<rep, period>;

    constexpr time() noexcept = default;
    constexpr explicit time(std::integral auto c) noexcept : value_{ static_cast<rep>(c) } { }

    template<typename Rep, typename Period>
    constexpr time(std::chrono::duration<Rep, Period> d)
        noexcept(std::is_nothrow_constructible_v<duration, std::chrono::duration<Rep, Period>>) :
        value_{ d }
    { }

    ////////////////////
    constexpr operator  gint64() const noexcept { return static_cast< gint64>(value_.count()); }
    constexpr operator guint64() const noexcept { return static_cast<guint64>(value_.count()); }

    auto operator&() { return proxy{ this, value_.count() }; }

    static const time none, snone;

    ////////////////////
    constexpr auto count() const noexcept { return value_.count(); }
    constexpr operator duration() const noexcept { return value_; }

    constexpr auto  operator+() const noexcept { return *this; }
    constexpr auto  operator-() const noexcept { return time{-value_}; }

    constexpr auto& operator++() noexcept { ++value_; return *this; }
    constexpr auto& operator--() noexcept { --value_; return *this; }

    constexpr auto  operator++(int) noexcept { return time{value_++}; }
    constexpr auto  operator--(int) noexcept { return time{value_--}; }

    constexpr auto& operator+=(time rhs) noexcept { value_ += rhs.value_; return *this; }
    constexpr auto& operator-=(time rhs) noexcept { value_ -= rhs.value_; return *this; }
    constexpr auto& operator%=(time rhs) noexcept { value_ %= rhs.value_; return *this; }

    constexpr auto& operator*=(rep  rhs) noexcept { value_ *= rhs; return *this; }
    constexpr auto& operator/=(rep  rhs) noexcept { value_ /= rhs; return *this; }
    constexpr auto& operator%=(rep  rhs) noexcept { value_ %= rhs; return *this; }

    friend constexpr auto operator+(time lhs, time rhs) noexcept { return lhs += rhs; }
    friend constexpr auto operator-(time lhs, time rhs) noexcept { return lhs -= rhs; }

    friend constexpr auto operator*(time lhs, std::integral auto rhs) noexcept { return lhs *= rhs; }
    friend constexpr auto operator*(std::integral auto lhs, time rhs) noexcept { return rhs *= lhs; }

    friend constexpr auto operator/(time lhs, std::integral auto rhs) noexcept { return lhs /= rhs; }
    friend constexpr auto operator/(time lhs, time rhs) noexcept { return lhs.value_ / rhs.value_; }

    friend constexpr auto operator%(time lhs, std::integral auto rhs) noexcept { return lhs %= rhs; }
    friend constexpr auto operator%(time lhs, time rhs) noexcept { return lhs %= rhs; }

    friend constexpr auto operator==(time lhs, time rhs) noexcept { return lhs.value_ == rhs.value_; }
    friend constexpr auto operator<=>(time lhs, time rhs) noexcept { return lhs.value_ <=> rhs.value_; }

    static constexpr auto zero() noexcept { return time{duration::zero()}; }
    static constexpr auto  min() noexcept { return time{duration:: min()}; }
    static constexpr auto  max() noexcept { return time{duration:: max()}; }

private:
    duration value_;

    struct proxy
    {
        time* owner;
        rep value;

        ~proxy() { owner->value_ = duration{value}; }

        operator  gint64*() { return reinterpret_cast< gint64*>(&value); }
        operator guint64*() { return reinterpret_cast<guint64*>(&value); }
    };
};

inline constexpr time time:: none{ GST_CLOCK_TIME_NONE  };
inline constexpr time time::snone{ GST_CLOCK_STIME_NONE };

////////////////////////////////////////////////////////////////////////////////
namespace literals
{
    constexpr auto operator""_h  (unsigned long long n) noexcept { return time{std::chrono::hours{n}}; }
    constexpr auto operator""_min(unsigned long long n) noexcept { return time{std::chrono::minutes{n}}; }
    constexpr auto operator""_s  (unsigned long long n) noexcept { return time{std::chrono::seconds{n}}; }
    constexpr auto operator""_ms (unsigned long long n) noexcept { return time{std::chrono::milliseconds{n}}; }
    constexpr auto operator""_us (unsigned long long n) noexcept { return time{std::chrono::microseconds{n}}; }
    constexpr auto operator""_ns (unsigned long long n) noexcept { return time{std::chrono::nanoseconds{n}}; }
}
using namespace literals;

////////////////////////////////////////////////////////////////////////////////
template<typename Dur, typename Rep, typename Period>
    requires std::same_as<Dur, time>
constexpr auto duration_cast(const std::chrono::duration<Rep, Period>& d)
{ return time{ std::chrono::duration_cast<typename time::duration>(d) }; }

template<typename Dur>
    requires (!std::same_as<Dur, time>)
constexpr auto duration_cast(const time& t)
{ return std::chrono::duration_cast<Dur>(static_cast<typename time::duration>(t)); }

}

////////////////////////////////////////////////////////////////////////////////
namespace std
{

template<>
struct formatter<pri::time> : formatter<pri::time::duration>
{
    auto format(const pri::time& t, format_context& ctx) const
    { return formatter<pri::time::duration>::format(static_cast<pri::time::duration>(t), ctx); }
};

}
