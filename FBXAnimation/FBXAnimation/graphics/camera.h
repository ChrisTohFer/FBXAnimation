#pragma once

#include "maths/geometry.h"

namespace graphics
{

    struct Camera
    {
        geom::Vector3 translation = { 0,2,3 };
        geom::Vector3 rotation_euler = geom::PI * geom::Vector3::unit_y();
        float aspect_ratio = 1.f;
        float fov = geom::PI * 0.5f;
        float near = 0.01f;
        float far = 100.f;

        geom::Vector3 facing() const;
        geom::Matrix44 calculate_camera_matrix() const;
    };

}