#pragma once

#include <cmath>
#include "Math.h"

namespace Cyrex::Math {
    class Vector4;
    class Vector2;
    class Quaternion;
    class Matrix;
    class Vector3 {
    public:
        Vector3() noexcept : x(0), y(0), z(0) {}
        Vector3(float x, float y, float z)  : x(x), y(y), z(0) {}
        Vector3(float v) noexcept : x(v), y(v), z(v) {}
        Vector3(const Vector3& rhs) noexcept {
            x = rhs.x;
            y = rhs.y;
            z = rhs.z;
        }
        Vector3(const Vector2& rhs) noexcept;
        Vector3(const Vector4& rhs) noexcept;
        Vector3(float vec[3]) noexcept {
            x = vec[0];
            y = vec[1];
            z = vec[2];
        }
        ~Vector3() = default;

        [[nodiscard]] const Vector3 operator+(const Vector3& rhs) const noexcept { 
            return Vector3(
                x + rhs.x, 
                y + rhs.y, 
                z + rhs.z
            ); 
        }
        [[nodiscard]] const Vector3 operator+(const float v) const noexcept { 
            return Vector3(x + v, y + v, z + v); 
        }
        void operator+=(const Vector3& rhs) noexcept {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
        }

        [[nodiscard]] const Vector3 operator*(const Vector3& rhs) const noexcept { 
            return Vector3(
                x * rhs.x,
                y * rhs.y, 
                z * rhs.z
            ); 
        }
        [[nodiscard]] const Vector3 operator*(const float value) const  noexcept { 
            return Vector3(
                x * value,
                y * value, 
                z * value
            ); 
        }

        void operator*=(const Vector3& other) noexcept {
            x *= other.x;
            y *= other.y;
            z *= other.z;
        }

        void operator*=(const float value) noexcept {
            x *= value;
            y *= value;
            z *= value;
        }

        [[nodiscard]] const Vector3 operator-(const Vector3 rhs) const noexcept { 
            return Vector3(
                x - rhs.x, 
                y - rhs.y, 
                z - rhs.z
            );
        }
        [[nodiscard]] const Vector3 operator-(const float value) const noexcept {
            return Vector3(
                x - value, 
                y - value, 
                z - value
            ); 
        }
        void operator-=(const Vector3 other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
        }

        [[nodiscard]] const Vector3 operator/(const Vector3& rhs) const noexcept {
            return Vector3(
                x / rhs.x,
                y / rhs.y,
                z / rhs.z); 
        }
        [[nodiscard]] const Vector3 operator/(const float value) const noexcept { 
            return Vector3(
                x / value, 
                y / value,
                z / value); 
        }

        void operator/=(const Vector3& other) noexcept {
            x /= other.x;
            y /= other.y;
            z /= other.y;
        }

        constexpr bool operator==(const Vector3& rhs) const noexcept { return x == rhs.x && y == rhs.y && z == rhs.z; }
        constexpr bool operator!=(const Vector3& rhs) const noexcept { return !(*this == rhs); }

        void Normalize() noexcept {
            const auto squaredLength = SquaredLength();

            if (!Math::Equals(squaredLength, 1.0f) && squaredLength > 0.0f) {
                const auto invertedLength = 1.0f / std::sqrt(squaredLength);
                
                x *= invertedLength;
                y *= invertedLength;
                z *= invertedLength;
            }
        }
        [[nodiscard]] const Vector3 Normalized() const noexcept {
            const auto squaredLength = SquaredLength();

            if (!Math::Equals(squaredLength, 1.0f) && squaredLength > 0.0f) {
                const auto invertedLength = 1.0f / std::sqrt(squaredLength);

                return (*this) * invertedLength;
            }
            return *this;
        }

        [[nodiscard]] constexpr float Dot(const Vector3& rhs) const noexcept { return Dot(*this, rhs); }
        [[nodiscard]] const Vector3 Cross(const Vector3& rhs) const noexcept { return Cross(*this, rhs); }

        [[nodiscard]] const float Length() const noexcept        { return std::sqrt(x * x + y * y + z * z); }
        [[nodiscard]] const float SquaredLength() const noexcept { return x * x + y * y + z * z; }

        inline void ClampMagnitude(float maxLength) noexcept {
            const auto squaredMagnitude = SquaredLength();

            if (squaredMagnitude > maxLength * maxLength) {
                const auto magnitude = std::sqrt(squaredMagnitude);

                const auto normalizedX = x / magnitude;
                const auto normalizedY = y / magnitude;
                const auto normalizedZ = z / magnitude;

                x = normalizedX * maxLength;
                y = normalizedY * maxLength;
                z = normalizedZ * maxLength;
            }
        }

        void Floor() noexcept {
            x = std::floor(x);
            y = std::floor(y);
            z = std::floor(z);
        }

        [[nodiscard]] Vector3 Abs() const noexcept { return Vector3(std::abs(x), std::abs(y), std::abs(z)); }

        [[nodiscard]] const inline float Distance(const Vector3& rhs)        const noexcept { return (*this - rhs).Length(); }
        [[nodiscard]] const inline float SquaredDistance(const Vector3& rhs) const noexcept { return (*this - rhs).SquaredLength(); }

        [[nodiscard]] static constexpr inline float Dot(const Vector3& v1, const Vector3& v2) noexcept {
            return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
        }
        [[nodiscard]] static const inline Vector3 Cross(const Vector3& v1, const Vector3& v2) noexcept {
            return Vector3(
                v1.y * v2.z - v2.y * v1.z,
                -(v1.x * v2.z - v2.x * v1.z),
                v1.x * v2.y - v2.x * v2.y
            );
        }
        [[nodiscard]] static const inline Vector3 Normalize(const Vector3& rhs) noexcept { return rhs.Normalized(); }
        [[nodiscard]] static const inline float Distance(const Vector3& a, const Vector3& b)        noexcept { return (b - a).Length(); }
        [[nodiscard]] static const inline float SquaredDistance(const Vector3& a, const Vector3& b) noexcept { return (b - a).SquaredLength(); }

        [[nodiscard]] static Vector3 Rotate(const Vector3& vec3, const Quaternion& rotation) noexcept;
        [[nodiscard]] static Vector3 TransformNormal(const Vector3& vec3, const Matrix& mat) noexcept;
        [[nodiscard]] static Vector3 TransformCoord(const Vector3& vec3, const Matrix& mat) noexcept;
        [[nodiscard]] static Vector3 MultiplyAdd(const Vector3& first, const Vector3& second, const Vector3& third) noexcept;

        float x;
        float y;
        float z;

        static const Vector3 Zero;
        static const Vector3 Left;
        static const Vector3 Right;
        static const Vector3 Up;
        static const Vector3 Down;
        static const Vector3 Forward;
        static const Vector3 Backward;
        static const Vector3 One;
        static const Vector3 Infinity;
        static const Vector3 InfinityNeg;
    };

    inline Vector3 operator*(float val, const Vector3& rhs) { return rhs * val; }
}