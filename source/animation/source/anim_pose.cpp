#include "anim_pose.h"

namespace anim
{
    //transform

    geom::Matrix44 Transform::calculate_matrix() const
    {
        return geom::create_translation_matrix_44(translation) * geom::create_rotation_matrix_from_quaternion(rotation);
    }

    //skeleton

    std::vector<geom::Matrix44> Skeleton::matrix_stack()
    {
        std::vector<geom::Matrix44> matrices;
        for (auto& bone : bones)
        {
            matrices.push_back(bone.global_transform.calculate_matrix());
        }
        return matrices;
    }

    //pose

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

    //animation

    void Animation::add_keyframe(Pose pose, float time)
    {
        //assume that keyframes will be added in order for now
        _ASSERT(time > m_duration);
        _ASSERT(pose.skeleton == &m_skeleton);
        m_duration = time;
        m_key_frames.push_back({ std::move(pose), time });
    }

    Pose Animation::get_pose(float time, bool loop) const
    {
        //if loop is enabled then ensure time is within duration
        time = loop ? fmodf(time, m_duration) : time;

        //if time is outside duration (only possible if not looping) then take first or final keyframe
        if (time <= 0)
        {
            return m_key_frames[0].pose;
        }
        if (time >= m_duration)
        {
            return m_key_frames.back().pose;
        }

        //interpolate between two adjacent keyframes
        int i = 1;
        float t = 0.f;
        for (; i < m_key_frames.size(); ++i)
        {
            float key_frame_time = m_key_frames[i].time;
            if (time < key_frame_time)
            {
                float prev_key_frame_time = m_key_frames[i - 1].time;
                t = (time - prev_key_frame_time) / (key_frame_time - prev_key_frame_time);
                break;
            }
        }

        return Pose::interpolate(m_key_frames[i - 1].pose, m_key_frames[i].pose, t);
    }
}