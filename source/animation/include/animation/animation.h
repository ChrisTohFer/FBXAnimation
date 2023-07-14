#pragma once

#include "pose.h"
#include "skeleton.h"

namespace anim
{
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