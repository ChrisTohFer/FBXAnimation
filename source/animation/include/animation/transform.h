#pragma once

#include "maths/vector3.h"
#include "maths/quaternion.h"

namespace anim
{
    using Translation = geom::Vector3;
    using Rotation = geom::Quaternion;

    //simple transform consisting of a vector3 translation and quaternion rotation
    struct Transform
    {
        Translation translation;
        Rotation rotation;

        geom::Matrix44 calculate_matrix() const;
    };

    bool operator==(const Transform&, const Transform&);
}