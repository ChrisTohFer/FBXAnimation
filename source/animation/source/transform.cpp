#include "transform.h"

#include "maths/geometry.h"

namespace anim
{
    geom::Matrix44 Transform::calculate_matrix() const
    {
        return geom::create_translation_matrix_44(translation) * geom::create_rotation_matrix_from_quaternion(rotation);
    }

    bool operator==(const Transform& lhs, const Transform& rhs)
    {
        return 
            lhs.rotation == rhs.rotation &&
            lhs.translation == rhs.translation;
    }
}