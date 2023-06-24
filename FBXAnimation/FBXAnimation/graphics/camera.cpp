#include "camera.h"

namespace graphics
{

    geom::Vector3 Camera::facing() const
    {
        return
            geom::create_z_rotation_matrix_44(rotation_euler.z) *
            geom::create_y_rotation_matrix_44(rotation_euler.y) *
            geom::create_x_rotation_matrix_44(rotation_euler.x) *
            geom::Vector3::unit_z();
    }
    geom::Matrix44 Camera::calculate_camera_matrix() const
    {
        return
            geom::create_projection_matrix_44(aspect_ratio, fov, near, far) *
            geom::create_x_rotation_matrix_44(-rotation_euler.x) *
            geom::create_y_rotation_matrix_44(-rotation_euler.y) *
            geom::create_z_rotation_matrix_44(-rotation_euler.z) *
            geom::create_translation_matrix_44(-translation);
    }

}