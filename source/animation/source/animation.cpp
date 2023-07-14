#include "animation.h"

namespace anim
{
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