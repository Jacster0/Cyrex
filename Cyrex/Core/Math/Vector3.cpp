#include "Vector3.h"
#include "Vector4.h"
#include "Vector2.h"
#include "Quaternion.h"
#include "Matrix.h"

namespace Cyrex::Math {
    constexpr auto infinity = std::numeric_limits<float>::infinity();

    Vector3::Vector3(const Vector4& rhs) noexcept 
        :
        x(rhs.x),
        y(rhs.y),
        z(rhs.z)
    {}

    Vector3::Vector3(const Vector2& rhs) noexcept
        :
        x(rhs.x),
        y(rhs.y),
        z(0)
    {}

    Vector3 Vector3::Rotate(const Vector3& vec3, const Quaternion& rotation) noexcept {
        const Quaternion p{ vec3.x, vec3.y, vec3.z, 0 };
        const Quaternion q  = rotation.Conjugate();

        const Quaternion& res = rotation * p * q;

        return Vector3(res.x, res.y, res.z);
    }

    Vector3 Vector3::TransformNormal(const Vector3& vec3, const Matrix& mat) noexcept {
        Vector3 row0{ mat.m00,mat.m01, mat.m02 };
        Vector3 row1{ mat.m10,mat.m11, mat.m12 };
        Vector3 row2{ mat.m20,mat.m21, mat.m22 };

        Vector3 X = Vector3(vec3.x);
        Vector3 Y = Vector3(vec3.y);
        Vector3 Z = Vector3(vec3.z);

        Vector3 Result = Z * row2;
        Result = MultiplyAdd(Y, row1, Result);
        Result = MultiplyAdd(X, row0, Result);

        return Result;
    }

    Vector3 Vector3::TransformCoord(const Vector3& vec3, const Matrix& mat) noexcept {
        Vector3 row0{ mat.m00,mat.m01, mat.m02 };
        Vector3 row1{ mat.m10,mat.m11, mat.m12 };
        Vector3 row2{ mat.m20,mat.m21, mat.m22 };
        Vector3 row3{ mat.m20,mat.m21, mat.m22 };

        Vector3 X = Vector3(vec3.x);
        Vector3 Y = Vector3(vec3.y);
        Vector3 Z = Vector3(vec3.z);

        Vector3 Result = MultiplyAdd(Z, row2, row3);
        Result         = MultiplyAdd(Y, row1, Result);
        Result         = MultiplyAdd(X, row0, Result);

        Vector3 W = Vector3(Result.z);
        return Result /  W;
    }

    Vector3 Vector3::MultiplyAdd(const Vector3& first, const Vector3& second, const Vector3& third) noexcept {
        return Vector3(first.x * second.x + third.x,
                       first.y * second.y + third.y,
                       first.z * second.z + third.z);
    }

    const Vector3 Vector3::Zero(0.0f, 0.0f, 0.0f);
    const Vector3 Vector3::One(1.0f, 1.0f, 1.0f);
    const Vector3 Vector3::Left(-1.0f, 0.0f, 0.0f);
    const Vector3 Vector3::Right(1.0f, 0.0f, 0.0f);
    const Vector3 Vector3::Up(0.0f, 1.0f, 0.0f);
    const Vector3 Vector3::Down(0.0f, -1.0f, 0.0f);
    const Vector3 Vector3::Forward(0.0f, 0.0f, 1.0f);
    const Vector3 Vector3::Backward(0.0f, 0.0f, -1.0f);
    const Vector3 Vector3::Infinity(infinity, infinity, infinity);
    const Vector3 Vector3::InfinityNeg(-infinity, -infinity, -infinity);
}