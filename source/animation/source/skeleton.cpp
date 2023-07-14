#include "skeleton.h"

namespace anim
{
    std::vector<geom::Matrix44> Skeleton::matrix_stack()
    {
        std::vector<geom::Matrix44> matrices;
        for (auto& bone : bones)
        {
            matrices.push_back(bone.global_transform.calculate_matrix());
        }
        return matrices;
    }

    bool Skeleton::equivalent(const Skeleton& lhs, const Skeleton& rhs)
    {
        if (lhs.name == rhs.name)
        {
            return false;
        }
        if (lhs.bones.size() != rhs.bones.size())
        {
            return false;
        }
        for (int i = 0; i < lhs.bones.size(); ++i)
        {
            auto& lhs_bone = lhs.bones[i];
            auto& rhs_bone = rhs.bones[i];
            if (lhs_bone.global_transform != rhs_bone.global_transform)
            {
                return false;
            }
            if (lhs_bone.parent_index != rhs_bone.parent_index)
            {
                return false;
            }
        }

        return true;
    }
}