#pragma once

#include <cmath>
#include <math.h>

namespace geom
{

    struct Vector3
    {
        float x;
        float y;
        float z;

        //useful default values
        static Vector3 zero()   { return { 0.f,0.f,0.f }; }
        static Vector3 one()    { return { 1.f,1.f,1.f }; }
        static Vector3 unit_x() { return { 1.f,0.f,0.f }; }
        static Vector3 unit_y() { return { 0.f,1.f,0.f }; }
        static Vector3 unit_z() { return { 0.f,0.f,1.f }; }

        //operations
        //+-*/ operations are defined as non-member functions
        static float dot(const Vector3&, const Vector3&);
        static Vector3 cross(const Vector3&, const Vector3&);
        static Vector3 interpolate(const Vector3&, const Vector3&, float t);

        float magnitude_squared() const;
        float magnitude() const;
        Vector3 normalized() const;
    };

    //non-member inline functions

    inline Vector3 operator-(const Vector3& value)
    {
        return {
            -value.x,
            -value.y,
            -value.z
        };
    }

    inline Vector3 operator+(const Vector3& lhs, const Vector3& rhs)
    {
        return {
            lhs.x + rhs.x,
            lhs.y + rhs.y,
            lhs.z + rhs.z
        };
    }
    inline Vector3 operator-(const Vector3& lhs, const Vector3& rhs)
    {
        return {
            lhs.x - rhs.x,
            lhs.y - rhs.y,
            lhs.z - rhs.z
        };
    }
    inline Vector3 operator*(const Vector3& lhs, float rhs)
    {
        return {
            lhs.x * rhs,
            lhs.y * rhs,
            lhs.z * rhs
        };
    }
    inline Vector3 operator*(float lhs, const Vector3& rhs)
    {
        return {
            rhs.x * lhs,
            rhs.y * lhs,
            rhs.z * lhs
        };
    }
    inline Vector3 operator/(const Vector3& lhs, float rhs)
    {
        return {
            lhs.x / rhs,
            lhs.y / rhs,
            lhs.z / rhs
        };
    }

    inline Vector3 operator+=(Vector3& lhs, const Vector3& rhs)
    {
        return lhs = lhs + rhs;
    }
    inline Vector3 operator-=(Vector3& lhs, const Vector3& rhs)
    {
        return lhs = lhs - rhs;
    }
    inline Vector3 operator*=(Vector3& lhs, float rhs)
    {
        return lhs = lhs * rhs;
    }
    inline Vector3 operator/=(Vector3& lhs, float rhs)
    {
        return lhs = lhs / rhs;
    }

    //inline member functions

    inline float Vector3::dot(const Vector3& lhs, const Vector3& rhs)
    {
        return
            lhs.x * rhs.x +
            lhs.y * rhs.y +
            lhs.z * rhs.z;
    }

    inline Vector3 Vector3::cross(const Vector3& lhs, const Vector3& rhs)
    {
        return {
            lhs.y * rhs.z - lhs.z * rhs.y,
            lhs.z * rhs.x - lhs.x * rhs.z,
            lhs.x * rhs.y - lhs.y * rhs.x
        };
    }

    inline Vector3 Vector3::interpolate(const Vector3& v1, const Vector3& v2, float t)
    {
        return {
            std::lerp(v1.x, v2.x, t),
            std::lerp(v1.y, v2.y, t),
            std::lerp(v1.z, v2.z, t)
        };
    }

    inline float Vector3::magnitude_squared() const
    {
        return x * x + y * y + z * z;
    }

    inline float Vector3::magnitude() const
    {
        return sqrtf(magnitude_squared());
    }

    inline Vector3 Vector3::normalized() const
    {
        float m = magnitude();
        if (m == 0)
        {
            return zero();
        }
        return *this / m;
    }

}