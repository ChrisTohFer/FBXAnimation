#pragma once

#include "constants.h"
#include <math.h>

namespace geom
{
    //forward declarations

    template<int rows, int columns>
    struct Matrix;
    struct Vector3;

    using Matrix44 = Matrix<4, 4>;
    using Matrix34 = Matrix<3, 4>;

    //struct

    struct Quaternion
    {
        float x;
        float y;
        float z;
        float w;

        static Quaternion identity() { return { 0,0,0,1.f }; }
        static Quaternion slerp(const Quaternion& q1, const Quaternion& q2, float t);
        static Quaternion from_rotation_matrix(const Matrix44& mat);

        Quaternion raised_to_power(float power) const;
        Quaternion normalized() const;
        Quaternion inverse() const;
        Vector3 axis() const;
        Vector3 axis_normalized() const;
        float angle() const;
        float mod_squared() const;
    };

    //operators

    Quaternion operator+(const Quaternion& lhs, const Quaternion& rhs);
    Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs);
}

//deliberately included after declarations to prevent circular dependency
#include "matrix.h"
#include "vector3.h"

//inline definitions
namespace geom
{
    //inline operator definitions

    inline Quaternion operator+(const Quaternion& lhs, const Quaternion& rhs)
    {
        return { 
            lhs.x + rhs.x,
            lhs.y + rhs.y,
            lhs.z + rhs.z,
            lhs.w + rhs.w
        };
    }

    inline Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs)
    {
        //lhs.x * i * rhs.(xi, yj, zk, w)
        Quaternion qx = {
            lhs.x * rhs.w,
            -lhs.x * rhs.z,
            lhs.x * rhs.y,
            -lhs.x * rhs.x
        };
        //lhs.y * j * rhs.(xi, yj, zk, w)
        Quaternion qy = {
            lhs.y * rhs.z,
            lhs.y * rhs.w,
            -lhs.y * rhs.x,
            -lhs.y * rhs.y
        };
        //lhs.z * k * rhs.(xi, yj, zk, w)
        Quaternion qz = {
            -lhs.z * rhs.y,
            lhs.z * rhs.x,
            lhs.z * rhs.w,
            -lhs.z * rhs.z
        };
        //lhs.w * rhs.(xi, yj, zk, w)
        Quaternion qw = {
            lhs.w * rhs.x,
            lhs.w * rhs.y,
            lhs.w * rhs.z,
            lhs.w * rhs.w
        };

        return qx + qy + qz + qw;
    }

    inline Quaternion operator*(const Quaternion& q, float f)
    {
        return{
            q.x * f,
            q.y * f,
            q.z * f,
            q.w * f
        };
    }

    //inline member definitions

    inline Quaternion Quaternion::slerp(const Quaternion& qa, const Quaternion& qb, float t)
    {
        //todo - is this the most effective way to allow >360 degree interpolation?
        Quaternion a_to_b = qa.inverse() * qb;
        float fraction_of_pi = a_to_b.angle() / PI;
        if (fraction_of_pi > 1.f)
        {
            return qa * a_to_b.raised_to_power(-t * (2.f - fraction_of_pi) / fraction_of_pi);
        }
        else
        {
            return qa * a_to_b.raised_to_power(t);
        }

        ////code taken from https://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/slerp/index.htm
        ////possibly want to inspect this later for better understanding
        //
        //Quaternion result;
        //
        //// Calculate angle between the inputs
        //float cosHalfTheta = qa.w * qb.w + qa.x * qb.x + qa.y * qb.y + qa.z * qb.z;
        //
        //// if qa=qb or qa=-qb then theta = 0 and we can return qa
        //if (fabsf(cosHalfTheta) >= 1.0f)
        //{
        //    result = qa;
        //    return result;
        //}
        //
        //// Calculate temporary values.
        //float halfTheta = acosf(cosHalfTheta);
        //float sinHalfTheta = sqrtf(1.0f - cosHalfTheta * cosHalfTheta);
        //
        //// if theta = 180 degrees then result is not fully defined
        //// we could rotate around any axis normal to qa or qb
        //if (fabsf(sinHalfTheta) < 0.001f) { // fabs is floating point absolute
        //    result.w = (qa.w * 0.5f + qb.w * 0.5f);
        //    result.x = (qa.x * 0.5f + qb.x * 0.5f);
        //    result.y = (qa.y * 0.5f + qb.y * 0.5f);
        //    result.z = (qa.z * 0.5f + qb.z * 0.5f);
        //    return result;
        //}
        //
        //float ratioA = sinf((1.f - t) * halfTheta) / sinHalfTheta;
        //float ratioB = sinf(t * halfTheta) / sinHalfTheta;
        //
        ////calculate Quaternion
        //result.w = (qa.w * ratioA + qb.w * ratioB);
        //result.x = (qa.x * ratioA + qb.x * ratioB);
        //result.y = (qa.y * ratioA + qb.y * ratioB);
        //result.z = (qa.z * ratioA + qb.z * ratioB);
        //return result;
    }

    inline Quaternion Quaternion::from_rotation_matrix(const Matrix44& m)
    {
        //copied from https://d3cw3dd2w32x2b.cloudfront.net/wp-content/uploads/2015/01/matrix-to-quat.pdf
        //the source uses row vectors, so the matrix defined here is a transposed version

        Quaternion q;
        float t;
        if (m.get(2, 2) < 0)
        {
            if (m.get(0, 0) > m.get(1, 1))
            {
                t = 1 + m.get(0, 0) - m.get(1, 1) - m.get(2, 2);
                q = { t, m.get(1, 0) + m.get(0, 1), m.get(0, 2) + m.get(2, 0), m.get(2, 1) - m.get(1, 2) };
            }
            else
            {
                t = 1 - m.get(0, 0) + m.get(1, 1) - m.get(2, 2);
                q = { m.get(1, 0) + m.get(0, 1), t, m.get(2, 1) + m.get(1, 2), m.get(0, 2) - m.get(2, 0) };
            }
        }
        else
        {
            if (m.get(0, 0) < -m.get(1, 1))
            {
                t = 1 - m.get(0, 0) - m.get(1, 1) + m.get(2, 2);
                q = { m.get(0, 2) + m.get(2, 0), m.get(2, 1) + m.get(1, 2), t, m.get(1, 0) - m.get(0, 1) };
            }
            else
            {
                t = 1 + m.get(0, 0) + m.get(1, 1) + m.get(2, 2);
                q = { m.get(2, 1) - m.get(1, 2), m.get(0, 2) - m.get(2, 0), m.get(1, 0) - m.get(0, 1), t };
            }
        }

        _ASSERT(t != 0.f);
        q = q * (0.5f / sqrtf(t));

        return q;
    }

    inline Quaternion Quaternion::raised_to_power(float power) const
    {
        float mod_squared_value = mod_squared();
        float half_angle = 0.5f * angle();
        geom::Vector3 axis = axis_normalized();

        float front_term = powf(mod_squared_value, power * 0.5f);
        float vector_part_magnitude = sinf(power * half_angle);

        return {
            front_term * axis.x * vector_part_magnitude,
            front_term * axis.y * vector_part_magnitude,
            front_term * axis.z * vector_part_magnitude,
            front_term * cosf(power * half_angle)
        };
    }

    inline Quaternion Quaternion::normalized() const
    {
        float mag_squared = x * x + y * y + z * z + w * w;
        _ASSERT(mag_squared != 0.f);

        float scaling = 1.f / sqrtf(mag_squared);
        return { x * scaling, y * scaling, z * scaling, w * scaling };
    }

    inline Quaternion Quaternion::inverse() const
    {
        return { -x, -y, -z, w };
    }

    inline Vector3 Quaternion::axis() const
    {
        return { x,y,z };
    }

    inline Vector3 Quaternion::axis_normalized() const
    {
        return axis().normalized();
    }

    inline float Quaternion::angle() const
    {
        return 2.f * acosf(fminf(w, 1.f));
    }

    inline float Quaternion::mod_squared() const
    {
        return x * x + y * y + z * z + w * w;
    }
}