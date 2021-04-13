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

    template<typename T>
    concept Number = (std::signed_integral<T> || std::floating_point<T>) && Arithmethic<T> && std::_Boolean_testable<T>;

    template<typename T>
    concept Unsigned_Number = std::unsigned_integral<T> && Arithmethic<T> && std::_Boolean_testable<T>;


    struct MathConstants {
        static constexpr auto EPSILON    = std::numeric_limits<float>::epsilon();
        static constexpr auto PI_FLOAT   = std::numbers::pi_v<float>;
        static constexpr auto PI         = PI_FLOAT;
        static constexpr auto PI_DIV2    = PI_FLOAT / 2.0f;
        static constexpr auto PI_MUL2    = PI_FLOAT * 2.0f;
        static constexpr auto PI_DOUBLE  = std::numbers::pi_v<double>;
        static constexpr auto PI_MUL2_D  = PI_DOUBLE * 2.0f;
        static constexpr auto PI_DIV2_D  = PI_DOUBLE * 2.0f;
        static constexpr auto TO_DEGREES = 180.0f / MathConstants::PI;
        static constexpr auto TO_RADIANS = MathConstants::PI / 180.0f;
        static constexpr auto MAX_LONG   = std::numeric_limits<long>::max;
    };

    //Check for equality but allow for a small error
    [[nodiscard]] inline constexpr bool Equals(Number auto lhs, Number auto rhs, float error = MathConstants::EPSILON) {
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
        float angle = (static_cast<float>(i) * MathConstants::PI_MUL2 / static_cast<float>(tesselation)) + MathConstants::PI_DIV2;
        float dx;
        float dz;

        DirectX::XMScalarSinCos(&dx, &dz, angle);

        DirectX::XMVECTORF32 vec = { { { dx, 0, dz, 0 } } };
        return vec;
    }

    [[nodiscard]] inline DirectX::XMVECTOR GetCircleVector(size_t i, size_t tesselation) noexcept {
        float angle = static_cast<float>(i) * MathConstants::PI_MUL2 / static_cast<float>(tesselation);
        float dx;
        float dz;

        DirectX::XMScalarSinCos(&dx, &dz, angle);

        DirectX::XMVECTORF32 vec = { dx, 0, dz, 0 };

        return vec;
    }

    [[nodiscard]] inline constexpr Number auto ToDegrees(const Number auto rads) noexcept {
        return rads * MathConstants::TO_DEGREES;
    }

    [[nodiscard]] inline constexpr Number auto ToRadians(const Number auto degrees) noexcept {
        return degrees * MathConstants::TO_RADIANS;
    }

    template<Unsigned_Number T>
    [[nodiscard]] inline constexpr T signum(T x) {
        return static_cast<T>(0) < x;
    }

    template<Number T>
    [[nodiscard]] inline constexpr T auto Signum(T val) {
        return (static_cast<T>(0) < val) - (val < static_cast<T>(0));
    }

    [[nodiscard]] inline const float Cot(float v) noexcept { return cos(v) / sin(v); }

    [[nodiscard]] inline constexpr auto GetNearestPow2(std::integral auto val, bool roundUp = true) noexcept {
        if (std::has_single_bit(val)) {
            return val;
        }

        const auto next = std::bit_ceil(val);
        const auto prev = std::bit_floor(val);

        const auto n = next - val;
        const auto m = val - prev;

        return (roundUp) ? ((n <= m) ? next : prev)
                         : ((n < m)  ? next : prev);
    }
}