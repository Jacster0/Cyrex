#pragma once
#include <numbers>

namespace Cyrex::Math {
    template<typename T>
    concept Addable = requires (T x) { x + x; };

    template<typename T>
    concept Subtractable = requires (T x) { x - x; };

    template<typename T>
    concept Divisble = requires (T x) { x / x; };

    template<typename T>
    concept Multipliable = requires (T x) { x * x; };

    template<typename T>
    concept MathematicallyOperational = Divisble<T> || Subtractable<T> || Divisble<T> || Multipliable<T>;

    struct MathConstants {
        static constexpr auto pi_float = std::numbers::pi_v<float>;
        static constexpr auto pi_div2 = pi_float / 2.0f;
        static constexpr auto pi_mul2 = pi_float * 2.0f;
    };

    template<typename T>
    inline T AlignUpWithMask(T value, size_t mask) {
        return static_cast<T>((static_cast<size_t>(value + mask) & ~mask));
    }

    template <typename T>
    inline T AlignDownWithMask(T value, size_t mask) {
        return static_cast<T>((static_cast<size_t>(value) & ~mask));
    }

    template <typename T>
    inline T AlignUp(T value, size_t alignment) {
        return AlignUpWithMask(value, alignment - 1);
    }

    template <typename T>
    inline T AlignDown(T value, size_t alignment) {
        return AlignDownWithMask(value, alignment - 1);
    }

    template<typename T>
    inline bool IsAligned(T value, size_t alignment) {
        return 0 == (static_cast<size_t>(value) & (alignment - 1));
    }

    template<Divisble T>
    inline T DivideByMultiple(T value, size_t alignment) {
        return (T)((value + alignment - 1) / alignment);
    }
}