#pragma once

#include <cmath>
#include "Math.h"

namespace Cyrex::Math {
    class Vector3;
    class Quaternion;
    class Matrix;
    class Vector4 {
    public:
        Vector4() noexcept : x(0), y(0), z(0), w(0) {}
        Vector4(float x, float y, float z, float w) noexcept : x(x), y(y), z(z), w(w) {}
        Vector4(float v) noexcept : x(v), y(v), z(v), w(v) {}
        Vector4(const Vector3& vec3, float w);
        Vector4(const Vector3& vec3);
        Vector4(const Quaternion& quat);
        ~Vector4() = default;

        [[nodiscard]] const bool operator ==(const Vector4 rhs) const noexcept {
            return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
        }
        [[nodiscard]] const bool operator !=(const Vector4 rhs) const noexcept {
            return !(*this == rhs);
        }

        [[nodiscard]] const Vector4 operator*(const float value) const noexcept {
            return Vector4(
                x * value,
                y * value,
                z * value,
                w * value
            );
        }

        void operator*=(const float value) noexcept {
            x *= value;
            y *= value;
            z *= value;
            w *= value;
        }
        const Vector4 operator*(const float val) noexcept {
            return Vector4(
                x * val,
                y * val,
                z * val,
                w * val
            );
        }
        
        [[nodiscard]] const Vector4 operator*(const Vector4& rhs) const noexcept {
            return Vector4(
                x * rhs.x,
                y * rhs.y,
                z * rhs.z,
                w * rhs.w
            );
        }
        void operator*=(const Vector4& rhs) noexcept {
            x *= rhs.x;
            y *= rhs.y;
            z *= rhs.z;
            w *= rhs.w;
        }

        [[nodiscard]] const Vector4 operator+(const float v) const noexcept {
            return Vector4(x + v, y + v, z + v, w);
        }

        [[nodiscard]] const Vector4 operator+(const Vector4& rhs) const noexcept {
            return Vector4(x + rhs.x, y + rhs.y, z + rhs.z, w);
        }
        void operator+=(const Vector4& rhs) noexcept {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
        }

        [[nodiscard]] const Vector4 operator/(const Vector4& rhs) const noexcept {
            return Vector4(
                x / rhs.x,
                y / rhs.y,
                z / rhs.z,
                w / rhs.w);
        }
        void operator/=(const Vector4& rhs) noexcept {
            x /= rhs.x;
            y /= rhs.y;
            z /= rhs.y;
            w /= rhs.w;
        }
        [[nodiscard]] const Vector4 operator /(const float val) const noexcept {
            return Vector4(x / val, y / val, z / val, w / val);
        }

        [[nodiscard]] const float Length()        const noexcept { return std::sqrt(x * x + y * y + z * z + w * w); }
        [[nodiscard]] const float SquaredLength() const noexcept { return x * x + y * y + z * z + w * w; }

        void Normalize() noexcept {
            const auto length_squared = SquaredLength();

            if (!Math::Equals(length_squared, 1.0f) && length_squared > 0.0f) {
                const auto length_inverted = 1.0f / std::sqrt(length_squared);
                x *= length_inverted;
                y *= length_inverted;
                z *= length_inverted;
                w *= length_inverted;
            }
        };
        [[nodiscard]] const Vector4 Normalized() const noexcept {
            const auto length_squared = SquaredLength();

            if (!Math::Equals(length_squared, 1.0f) && length_squared > 0.0f) {
                const auto length_inverted = 1.0f / std::sqrt(length_squared);

                return (*this) * length_inverted;
            }
            return *this;
        }

        [[nodiscard]] static const Vector4 Normalize(const Vector4 rhs) noexcept { return rhs.Normalized(); };
        [[nodiscard]] static const Vector4 Negate(const Vector4 vec4) noexcept { return Vector4(-vec4.x, -vec4.y, -vec4.z, vec4.w); }
        [[nodiscard]] static Vector4 Rotate(const Vector4& vec4, const Quaternion& rotation) noexcept;
        [[nodiscard]] static Vector4 TransformNormal(const Vector4& vec4, const Matrix& mat) noexcept;
        [[nodiscard]] static Vector4 TransformCoord(const Vector4& vec4, const Matrix& mat) noexcept;
        [[nodiscard]] static Vector4 Transform(const Vector4& vec4, const Matrix& mat) noexcept;
        [[nodiscard]] static Vector4 MultiplyAdd(const Vector4& first, const Vector4& second, const Vector4& third) noexcept;

        float x;
        float y;
        float z;
        float w;

        static const Vector4 One;
        static const Vector4 Zero;
        static const Vector4 Infinity;
        static const Vector4 InfinityNeg;
    };
}