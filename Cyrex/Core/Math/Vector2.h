#pragma once
#include <cmath>

namespace Cyrex::Math {
    class Vector2 {
    public:
        Vector2() noexcept : x(0), y(0) {}
        Vector2(float x, float y) noexcept : x(x), y(y) {}
        Vector2(int x, int y) noexcept : x(static_cast<float>(x)), y(static_cast<float>(y)) {}
        Vector2(uint32_t x, uint32_t y) noexcept : x(static_cast<float>(x)), y(static_cast<float>(y)) {}
        Vector2(float x) noexcept : x(x), y(x) {}
        Vector2(const Vector2& rhs) noexcept {
            x = rhs.x;
            y = rhs.y;
        }
        ~Vector2() = default;

        [[nodiscard]] const Vector2 operator+(const Vector2& other) const noexcept { return Vector2(x + other.x, y + other.y); }
        void operator+=(const Vector2& other) noexcept {
            x += other.x; 
            y += other.y; 
        }

        [[nodiscard]] const Vector2 operator*(const Vector2& other) const noexcept { return Vector2(x * other.x, y * other.y); }
        [[nodiscard]] const Vector2 operator*(const float value)    const noexcept { return Vector2(x * value, y * value); }

        void operator*=(const Vector2& other) {
            x *= other.x;
            y *= other.y;
        }

        void operator*=(const float value) {
            x *= value;
            y *= value;
        }
        [[nodiscard]] const Vector2 operator-(const Vector2 other) const noexcept { return Vector2(x - other.x, y - other.y); }
        [[nodiscard]] const Vector2 operator-(const float value)   const noexcept { return Vector2(x - value, y - value); }
        void operator-=(const Vector2 other) { 
            x -= other.x; 
            y -= other.y;
        }

        [[nodiscard]] const Vector2 operator/(const Vector2& other) const noexcept { return Vector2(x / other.x, y / other.y); }
        [[nodiscard]] const Vector2 operator/(const float value) const noexcept { return Vector2(x / value, y / value); }

        void operator/=(const Vector2& other) noexcept {
            x /= other.x;
            y /= other.y;
        }

        constexpr bool operator==(const Vector2& other) const noexcept { return x == other.x && y == other.y; }
        constexpr bool operator!=(const Vector2& other) const noexcept { return !(*this == other); }

        [[nodiscard]] const float Length() const noexcept { return std::sqrt(x * x + y * y); }
        [[nodiscard]] const float SquaredLength() const noexcept { return x * x + y * y; }

        [[nodiscard]] static const inline float Distance(const Vector2& a, const Vector2& b) noexcept { return (b - a).Length(); }
        [[nodiscard]] static const inline float SquaredDistance(const Vector2& a, const Vector2& b) noexcept { return (b - a).SquaredLength(); }

        float x;
        float y;

        static const Vector2 Zero;
        static const Vector2 one;
    };
}