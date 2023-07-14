#include "pose.h"

#include "skeleton.h"

namespace anim
{
    Pose Pose::interpolate(const Pose& p1, const Pose& p2, float t)
    {
        _ASSERT(p1.local_transforms.size() == p2.local_transforms.size());
        _ASSERT(p1.skeleton == p2.skeleton);

        Pose interpolated_pose;
        interpolated_pose.skeleton = p1.skeleton;
        interpolated_pose.local_transforms.resize(p1.local_transforms.size());

        for (int i = 0; i < p1.local_transforms.size(); ++i)
        {
            Transform& pose_t = interpolated_pose.local_transforms[i];
            pose_t.translation = geom::Vector3::interpolate(
                p1.local_transforms[i].translation,
                p2.local_transforms[i].translation,
                t);

            pose_t.rotation = geom::Quaternion::slerp(p1.local_transforms[i].rotation, p2.local_transforms[i].rotation, t);
        }

        return interpolated_pose;
    }

    std::vector<geom::Matrix44> Pose::get_matrix_stack() const
    {
        std::vector<geom::Matrix44> stack;
        stack.resize(local_transforms.size());

        //do root first
        stack[0] = local_transforms[0].calculate_matrix();

        for (int i = 1; i < local_transforms.size(); ++i)
        {
            auto& parent_matrix = stack[skeleton->bones[i].parent_index];
            stack[i] = parent_matrix * local_transforms[i].calculate_matrix();
        }

        return stack;
    }
}