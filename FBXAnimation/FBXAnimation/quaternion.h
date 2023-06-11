#pragma once

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

    Quaternion Quaternion::normalized() const
    {
        float scaling = 1.f / sqrtf(x * x + y * y + z * z + w * w);
        return { x * scaling, y * scaling, z * scaling, w * scaling };
    }
}