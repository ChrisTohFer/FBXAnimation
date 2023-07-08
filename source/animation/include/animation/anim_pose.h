#pragma once

#include "maths/geometry.h"

#include <vector>

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

    //node hierarchy, transforms, and inverse transforms
    struct Skeleton
    {
        struct Bone
        {
            int parent_index;
            Transform global_transform;
        };

        std::vector<Bone> bones;
        std::vector<geom::Matrix44> inv_matrix_stack;
        
        std::vector<geom::Matrix44> matrix_stack();
    };

    //a collection of local transforms that represent a pose for a referenced skeleton 
    struct Pose
    {
        const Skeleton* skeleton;
        std::vector<Transform> local_transforms;

        static Pose interpolate(const Pose&, const Pose&, float t);
        std::vector<geom::Matrix44> get_matrix_stack() const;
    };

    //a collection of timestamped poses (keyframes) that can be sampled for a pose using a time parameter
    class Animation
    {
    public:
        Animation(const Skeleton& skeleton) : m_skeleton(skeleton) {}
        void add_keyframe(Pose, float time);

        float duration() const { return m_duration; }
        Pose get_pose(float time, bool loop = false) const;

    private:
        struct KeyFrame
        {
            Pose pose;
            float time;
        };
        const Skeleton& m_skeleton;
        std::vector<KeyFrame> m_key_frames;
        float m_duration = -1.f;
    };
}