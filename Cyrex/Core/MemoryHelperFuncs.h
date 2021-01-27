#pragma once

namespace Cyrex {
    constexpr inline auto KB(size_t size) noexcept { return size * 1024; }
    constexpr inline auto MB(size_t size) noexcept { return KB(size) * 1024; }

    static constexpr auto _64KB  = KB(64);
    static constexpr auto _1MB   = MB(1);
    static constexpr auto _2MB   = MB(2);
    static constexpr auto _4MB   = MB(4);
    static constexpr auto _8MB   = MB(8);
    static constexpr auto _16MB  = MB(16);
    static constexpr auto _32MB  = MB(32);
    static constexpr auto _64MB  = MB(64);
    static constexpr auto _128MB = MB(128);
    static constexpr auto _256MB = MB(256);
}

