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
    };

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
}