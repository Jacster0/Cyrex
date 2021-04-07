#pragma once
#include <random>
#include <concepts>
#include <algorithm>

namespace Cyrex::Math {
    class Rng {
    public:
        template<typename T>
        requires std::integral<T>
        [[nodiscard]] static constexpr inline auto Random(T low, T high) noexcept {
            std::default_random_engine engine{ std::random_device{}() };

            //This should never happen but we will support it anyway
            if (high < low) [[unlikely]] {
                std::swap(low, high);
            }

            return std::uniform_int_distribution<T>{ low, high}(engine);
        }

        template<typename T>
        requires std::floating_point<T> 
        [[nodiscard]] static constexpr inline auto Random(T low, T high) noexcept {
            std::default_random_engine engine{ std::random_device{}() };
 
            //This should never happen but we will support it anyway
            if (high < low) [[unlikely]] {
                std::swap(low, high);
            }
            return std::uniform_real_distribution<T>{ low, high}(engine);
        }
    };
}