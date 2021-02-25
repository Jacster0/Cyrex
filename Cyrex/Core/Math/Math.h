#pragma once
#include <numbers>
#include <DirectXMath.h>

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
    concept Arithmethic = Divisble<T> || Subtractable<T> || Divisble<T> || Multipliable<T>;

    struct MathConstants {
        static constexpr auto pi_float = std::numbers::pi_v<float>;
        static constexpr auto pi_div2 = pi_float / 2.0f;
        static constexpr auto pi_mul2 = pi_float * 2.0f;
    };

    template<typename T>
    inline T AlignUpWithMask(T value, size_t mask) noexcept {
        return static_cast<T>((static_cast<size_t>(value + mask) & ~mask));
    }

    template <typename T>
    inline T AlignDownWithMask(T value, size_t mask) noexcept {
        return static_cast<T>((static_cast<size_t>(value) & ~mask));
    }

    template <typename T>
    inline T AlignUp(T value, size_t alignment) noexcept {
        return AlignUpWithMask(value, alignment - 1);
    }

    template <typename T>
    inline T AlignDown(T value, size_t alignment) noexcept  {
        return AlignDownWithMask(value, alignment - 1);
    }

    template<typename T>
    inline bool IsAligned(T value, size_t alignment) noexcept  {
        return 0 == (static_cast<size_t>(value) & (alignment - 1));
    }

    auto DivideByMultiple(Divisble auto value, Divisble auto alignment) {
        return (value + alignment - 1) / alignment;
    }

    inline DirectX::XMVECTOR GetCircleTangent(size_t i, size_t tesselation) noexcept {
        float angle = (static_cast<float>(i) * MathConstants::pi_mul2 / static_cast<float>(tesselation)) + MathConstants::pi_div2;
        float dx;
        float dz;

        DirectX::XMScalarSinCos(&dx, &dz, angle);

        DirectX::XMVECTORF32 vec = { { { dx, 0, dz, 0 } } };
        return vec;
    }

    inline DirectX::XMVECTOR GetCircleVector(size_t i, size_t tesselation) noexcept {
        float angle = static_cast<float>(i) * MathConstants::pi_mul2 / static_cast<float>(tesselation);
        float dx;
        float dz;

        DirectX::XMScalarSinCos(&dx, &dz, angle);

        DirectX::XMVECTORF32 vec = { { { dx, 0, dz, 0 } } };

        return vec;
    }
}