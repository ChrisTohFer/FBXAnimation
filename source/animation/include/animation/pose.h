#pragma once

#include "transform.h"

#include <vector>

namespace anim
{
    class Skeleton;

    //a collection of local transforms that represent a pose for a referenced skeleton 
    struct Pose
    {
        const Skeleton* skeleton;
        std::vector<Transform> local_transforms;

        static Pose interpolate(const Pose&, const Pose&, float t);
        std::vector<geom::Matrix44> get_matrix_stack() const;
    };
}