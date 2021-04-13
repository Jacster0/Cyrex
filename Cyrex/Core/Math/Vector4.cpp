#include "Vector4.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix.h"

namespace Cyrex::Math {
    Vector4::Vector4(const Vector3& vec3, float w)
        :
        x(vec3.x),
        y(vec3.y),
        z(vec3.z),
        w(w)
    {}

    Vector4::Vector4(const Vector3& vec3)
        :
        x(vec3.x),
        y(vec3.y),
        z(vec3.z),
        w(0.0f)
    {}

    Vector4::Vector4(const Quaternion& quat)
        :
        x(quat.x),
        y(quat.y),
        z(quat.z),
        w(quat.w)
    {}

    Vector4 Vector4::Rotate(const Vector4& vec4, const Quaternion& rotation) noexcept {
        const Quaternion p{ vec4.x, vec4.y, vec4.z, 0 };
        const Quaternion q = rotation.Conjugate();

        const Quaternion& res = Quaternion::Multiply(Quaternion::Multiply(rotation,p), q);

        return Vector4(res);
    }

    Vector4 Vector4::TransformNormal(const Vector4& vec4, const Matrix& mat) noexcept {
        Vector4 row0{ mat.m00,mat.m01, mat.m02, mat.m03 };
        Vector4 row1{ mat.m10,mat.m11, mat.m12, mat.m13 };
        Vector4 row2{ mat.m20,mat.m21, mat.m22, mat.m23 };

        Vector4 X = Vector4(vec4.x);
        Vector4 Y = Vector4(vec4.y);
        Vector4 Z = Vector4(vec4.z);

        Vector4 Result = Z * row2;
        Result = MultiplyAdd(Y, row1, Result);
        Result = MultiplyAdd(X, row0, Result);
        Result.w = 0;

        return Result;
    }

    Vector4 Vector4::Transform(const Vector4& vec4, const Matrix& mat) noexcept {
        Vector4 row0{ mat.m00,mat.m01, mat.m02, mat.m03 };
        Vector4 row1{ mat.m10,mat.m11, mat.m12, mat.m13 };
        Vector4 row2{ mat.m20,mat.m21, mat.m22, mat.m23 };
        Vector4 row3{ mat.m30,mat.m31, mat.m32 ,mat.m33 };

        Vector4 X = Vector4(vec4.x);
        Vector4 Y = Vector4(vec4.y);
        Vector4 Z = Vector4(vec4.z);

        Vector4 Result = MultiplyAdd(Z, row2, row3);
        Result = MultiplyAdd(Y, row1, Result);
        Result = MultiplyAdd(X, row0, Result);
        Result.w = 0;

        return Result;
    }


    Vector4 Vector4::TransformCoord(const Vector4& vec3, const Matrix& mat) noexcept {
        Vector4 row0{ mat.m00,mat.m01, mat.m02, mat.m03 };
        Vector4 row1{ mat.m10,mat.m11, mat.m12, mat.m13 };
        Vector4 row2{ mat.m20,mat.m21, mat.m22, mat.m23 };
        Vector4 row3{ mat.m30,mat.m31, mat.m32 ,mat.m33 };

        Vector4 X = Vector4(vec3.x);
        Vector4 Y = Vector4(vec3.y);
        Vector4 Z = Vector4(vec3.z);

        Vector4 Result = MultiplyAdd(Z, row2, row3);
        Result         = MultiplyAdd(Y, row1, Result);
        Result         = MultiplyAdd(X, row0, Result);

        Vector4 W = Vector4(Result.w);
        return Result / W;
    }

    Vector4 Vector4::MultiplyAdd(const Vector4& first, const Vector4& second, const Vector4& third) noexcept {
        return Vector4(first.x  * second.x + third.x,
                        first.y * second.y + third.y,
                        first.z * second.z + third.z,
                        first.w * second.w + third.w);
    }
    constexpr auto inf = std::numeric_limits<float>::infinity();

    const Vector4 Vector4::One(1.0f, 1.0f, 1.0f, 1.0f);
    const Vector4 Vector4::Zero(0.0f, 0.0f, 0.0f, 0.0f);
    const Vector4 Vector4::Infinity(inf, inf, inf, inf);
    const Vector4 Vector4::InfinityNeg(-inf, -inf, -inf, -inf);
}
