#pragma once

#include <math.h>

namespace geom
{
    struct Quaternion
    {
        float x;
        float y;
        float z;
        float w;

        static Quaternion identity() { return { 0,0,0,1.f }; }
        Quaternion normalized() const;
        Quaternion inverse() const;
        float angle() const;
    };

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
            -lhs.z * rhs.x
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

    //inline definitions

    inline Quaternion operator*(const Quaternion& q, float f)
    {
        return{
            q.x * f,
            q.y * f,
            q.z * f,
            q.w * f
        };
    }

    inline Quaternion Quaternion::normalized() const
    {
        float scaling = 1.f / sqrtf(x * x + y * y + z * z + w * w);
        return { x * scaling, y * scaling, z * scaling, w * scaling };
    }

    inline Quaternion Quaternion::inverse() const
    {
        return { -x, -y, -z, w };
    }

    inline float Quaternion::angle() const
    {
        return 2 * acosf(w);
    }
}