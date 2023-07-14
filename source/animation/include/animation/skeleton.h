#pragma once

#include "pose.h"
#include "transform.h"

#include <string>
#include <vector>

namespace anim
{
    //node hierarchy, transforms, and inverse transforms
    struct Skeleton
    {
        struct Bone
        {
            int parent_index;
            Transform global_transform;
        };

        std::string name;
        std::vector<Bone> bones;
        std::vector<geom::Matrix44> inv_matrix_stack;

        std::vector<geom::Matrix44> matrix_stack();
        static bool equivalent(const Skeleton&, const Skeleton&);
    };
}