#pragma once
#include <numbers>
#include <DirectXMath.h>
#include <concepts>
#include <limits>
#include <bit>

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
        static constexpr auto epsilon   = std::numeric_limits<float>::epsilon();
        static constexpr auto pi_float  = std::numbers::pi_v<float>;
        static constexpr auto pi        = pi_float;
        static constexpr auto pi_div2   = pi_float / 2.0f;
        static constexpr auto pi_mul2   = pi_float * 2.0f;
        static constexpr auto pi_double = std::numbers::pi_v<double>;
        static constexpr auto pi_mul2_d = pi_double * 2.0f;
        static constexpr auto pi_div2_d = pi_double * 2.0f;
    };

    //Check for equality but allow for a small error
    template<typename T>
    requires Addable<T> && Subtractable<T> && std::_Boolean_testable<T>
    [[nodiscard]] constexpr inline bool Equals(T lhs, T rhs, T error = MathConstants::epsilon) {
        return lhs + error >= rhs && lhs - error <= rhs;
    }

    template<typename T>
    [[nodiscard]] inline T AlignUpWithMask(T value, size_t mask) noexcept {
        return static_cast<T>((static_cast<size_t>(value + mask) & ~mask));
    }

    template <typename T>
    [[nodiscard]] inline T AlignDownWithMask(T value, size_t mask) noexcept {
        return static_cast<T>((static_cast<size_t>(value) & ~mask));
    }

    template <typename T>
    [[nodiscard]] inline T AlignUp(T value, size_t alignment) noexcept {
        return AlignUpWithMask(value, alignment - 1);
    }

    template <typename T>
    [[nodiscard]] inline T AlignDown(T value, size_t alignment) noexcept  {
        return AlignDownWithMask(value, alignment - 1);
    }

    template<typename T>
    [[nodiscard]] inline bool IsAligned(T value, size_t alignment) noexcept  {
        return 0 == (static_cast<size_t>(value) & (alignment - 1));
    }

    [[nodiscard]] auto DivideByMultiple(Divisble auto value, Divisble auto alignment) {
        return (value + alignment - 1) / alignment;
    }

    [[nodiscard]] inline DirectX::XMVECTOR GetCircleTangent(size_t i, size_t tesselation) noexcept {
        float angle = (static_cast<float>(i) * MathConstants::pi_mul2 / static_cast<float>(tesselation)) + MathConstants::pi_div2;
        float dx;
        float dz;

        DirectX::XMScalarSinCos(&dx, &dz, angle);

        DirectX::XMVECTORF32 vec = { { { dx, 0, dz, 0 } } };
        return vec;
    }

    [[nodiscard]] inline DirectX::XMVECTOR GetCircleVector(size_t i, size_t tesselation) noexcept {
        float angle = static_cast<float>(i) * MathConstants::pi_mul2 / static_cast<float>(tesselation);
        float dx;
        float dz;

        DirectX::XMScalarSinCos(&dx, &dz, angle);

        DirectX::XMVECTORF32 vec = { dx, 0, dz, 0 };

        return vec;
    }

    template <typename T>
    requires (std::signed_integral<T> || std::floating_point<T>) && Multipliable<T>
    [[nodiscard]] constexpr inline T ToDegrees(const T rads) noexcept {
        return rads * (180.0f / MathConstants::pi);
    }

    template <typename T>
    requires (std::signed_integral<T> || std::floating_point<T>) && Multipliable<T>
    [[nodiscard]] constexpr inline T ToRadians(const T degrees) noexcept {
        return degrees * (MathConstants::pi / 180.0f);
    }

    template<typename T> 
    requires std::unsigned_integral<T> && (Subtractable<T> && std::_Boolean_testable<T>)
    [[nodiscard]] inline constexpr T signum(T x) {
        return static_cast<T>(0) < x;
    }

    template <typename T>
    requires (std::signed_integral<T> || std::floating_point<T>) && (Subtractable<T> && std::_Boolean_testable<T>)
    [[nodiscard]] inline constexpr T Signum(T val) {
        return (static_cast<T>(0) < val) - (val < static_cast<T>(0));
    }

    [[nodiscard]] inline const float Cot(float v) noexcept { return cos(v) / sin(v); }

    template <typename T>
    requires std::integral<T>
    [[nodiscard]] inline constexpr T GetNearestPow2(T v, bool roundUp = true) noexcept { 
        if (std::has_single_bit(v)) {
            return v;
        }

        const T next = std::bit_ceil(v);
        const T prev = std::bit_floor(v);

        const T n = next - v;
        const T m = v - prev;

        return (roundUp) ? ((n <= m) ? next : prev)
                         : ((n < m)  ? next : prev);
    }
}