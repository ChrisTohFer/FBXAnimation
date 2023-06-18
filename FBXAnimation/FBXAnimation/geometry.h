#pragma once

#include "vector3.h"
#include "matrix.h"
#include "quaternion.h"

//this file handles combined functionality between vectors, matrices, quaternions

namespace geom
{
    //operations

    Vector3 operator*(const Matrix44& mat, Vector3 vec)
    {
        Vector3 result;

        result.x =
            mat.get(0, 0) * vec.x +
            mat.get(0, 1) * vec.y +
            mat.get(0, 2) * vec.z +
            mat.get(0, 3);
        result.y =
            mat.get(1, 0) * vec.x +
            mat.get(1, 1) * vec.y +
            mat.get(1, 2) * vec.z +
            mat.get(1, 3);
        result.z =
            mat.get(2, 0) * vec.x +
            mat.get(2, 1) * vec.y +
            mat.get(2, 2) * vec.z +
            mat.get(2, 3);

        return result;
    }

    Vector3 operator*(const Matrix34& mat, Vector3 vec)
    {
        Vector3 result;

        result.x =
            mat.get(0, 0) * vec.x +
            mat.get(0, 1) * vec.y +
            mat.get(0, 2) * vec.z +
            mat.get(0, 3);
        result.y =
            mat.get(1, 0) * vec.x +
            mat.get(1, 1) * vec.y +
            mat.get(1, 2) * vec.z +
            mat.get(1, 3);
        result.z =
            mat.get(2, 0) * vec.x +
            mat.get(2, 1) * vec.y +
            mat.get(2, 2) * vec.z +
            mat.get(2, 3);

        return result;
    }

    //conversions/constructions

    Matrix34 create_translation_matrix_34(Vector3 vec)
    {
        auto result = Matrix34::identity();
        result.get(0, 3) = vec.x;
        result.get(1, 3) = vec.y;
        result.get(2, 3) = vec.z;
        return result;
    }
    Matrix44 create_translation_matrix_44(Vector3 vec)
    {
        auto result = Matrix44::identity();
        result.get(0, 3) = vec.x;
        result.get(1, 3) = vec.y;
        result.get(2, 3) = vec.z;
        return result;
    }
    Matrix44 create_scale_matrix_44(Vector3 vec)
    {
        auto result = Matrix44::identity();
        result.get(0, 0) = vec.x;
        result.get(1, 1) = vec.y;
        result.get(2, 2) = vec.z;
        return result;
    }
    Matrix44 create_x_rotation_matrix_44(float angle)
    {
        auto result = Matrix44::identity();

        result.get(0, 0) = 1.f;
        result.get(1, 0) = 0.f;
        result.get(2, 0) = 0.f;

        result.get(0, 1) = 0.f;
        result.get(1, 1) = cosf(angle);
        result.get(2, 1) = sinf(angle);

        result.get(0, 2) = 0.f;
        result.get(1, 2) = -sinf(angle);
        result.get(2, 2) = cosf(angle);

        return result;
    }
    Matrix44 create_y_rotation_matrix_44(float angle)
    {
        auto result = Matrix44::identity();

        result.get(0, 0) = cosf(angle);
        result.get(1, 0) = 0.f;
        result.get(2, 0) = -sinf(angle);

        result.get(0, 1) = 0.f;
        result.get(1, 1) = 1.f;
        result.get(2, 1) = 0.f;

        result.get(0, 2) = sinf(angle);
        result.get(1, 2) = 0.f;
        result.get(2, 2) = cosf(angle);

        return result;
    }
    Matrix44 create_z_rotation_matrix_44(float angle)
    {
        auto result = Matrix44::identity();

        result.get(0, 0) = cosf(angle);
        result.get(1, 0) = sinf(angle);
        result.get(2, 0) = 0.f;

        result.get(0, 1) = -sinf(angle);
        result.get(1, 1) = cosf(angle);
        result.get(2, 1) = 0.f;

        result.get(0, 2) = 0.f;
        result.get(1, 2) = 0.f;
        result.get(2, 2) = 1.f;

        return result;
    }
    Matrix44 create_rotation_matrix_from_quaternion(const Quaternion& q)
    {
        Matrix44 result;

        //this matrix may be the wrong way around

        result.get(0, 0) = 2.f * (q.w * q.w + q.x * q.x) - 1;
        result.get(0, 1) = 2.f * (q.x * q.y - q.w * q.z);
        result.get(0, 2) = 2.f * (q.x * q.z + q.w * q.y);
        result.get(0, 3) = 0.f;

        result.get(1, 0) = 2.f * (q.x * q.y + q.w * q.z);
        result.get(1, 1) = 2.f * (q.w * q.w + q.y * q.y) - 1;
        result.get(1, 2) = 2.f * (q.y * q.z - q.w * q.x);
        result.get(1, 3) = 0.f;

        result.get(2, 0) = 2.f * (q.x * q.z - q.w * q.y);
        result.get(2, 1) = 2.f * (q.y * q.z + q.w * q.x);
        result.get(2, 2) = 2.f * (q.w * q.w + q.z * q.z) - 1;
        result.get(2, 3) = 0.f;

        result.get(3, 0) = 0.f;
        result.get(3, 1) = 0.f;
        result.get(3, 2) = 0.f;
        result.get(3, 3) = 1.f;

        return result;
    }
    Quaternion create_quaternion_from_rotation_matrix(const Matrix44& m)
    {
        //copied from https://d3cw3dd2w32x2b.cloudfront.net/wp-content/uploads/2015/01/matrix-to-quat.pdf
        //may need some testing

        Quaternion q;
        float t;
        if (m.get(2,2) < 0)
        {
            if (m.get(0, 0) > m.get(1, 1))
            {
                t = 1 + m.get(0, 0) - m.get(1, 1) - m.get(2, 2);
                q = {t, m.get(0, 1) + m.get(1, 0), m.get(2, 0) + m.get(0, 2), m.get(1, 2) - m.get(2, 1)};
            }
            else
            {
                t = 1 - m.get(0, 0) + m.get(1, 1) - m.get(2, 2);
                q = {m.get(0, 1) + m.get(1, 0), t, m.get(1, 2) + m.get(2, 1), m.get(2, 0) - m.get(0, 2)};
            }
        }
        else
        {
            if (m.get(0, 0) < -m.get(1, 1))
            {
                t = 1 - m.get(0, 0) - m.get(1, 1) + m.get(2, 2);
                q = {m.get(2, 0) + m.get(0, 2), m.get(1, 2) + m.get(2, 1), t, m.get(0, 1) - m.get(1, 0)};
            }
            else
            {
                t = 1 + m.get(0, 0) + m.get(1, 1) + m.get(2, 2);
                q = {m.get(1, 2) - m.get(2, 1), m.get(2, 0) - m.get(0, 2), m.get(0, 1) - m.get(1, 0), t};
            }
        }
        q = q * (0.5f / sqrtf(t));

        return q;
    }

    Matrix44 create_projection_matrix_44(float aspect, float fov, float near, float far)
    {
        Matrix44 result;
        
        float tan_fov_2 = tanf(fov / 2.f);

        result.get(0, 0) = 1.f / (aspect * tan_fov_2);
        result.get(0, 1) = 0.f;
        result.get(0, 2) = 0.f;
        result.get(0, 3) = 0.f;
        
        result.get(1, 0) = 0.f;
        result.get(1, 1) = 1.f / tan_fov_2;
        result.get(1, 2) = 0.f;
        result.get(1, 3) = 0.f;
        
        result.get(2, 0) = 0.f;
        result.get(2, 1) = 0.f;
        result.get(2, 2) = (far + near) / (far - near);
        result.get(2, 3) = -2.f * far * near / (far - near);
        
        result.get(3, 0) = 0.f;
        result.get(3, 1) = 0.f;
        result.get(3, 2) = 1.f;
        result.get(3, 3) = 0.f;

        return result;
    }

    Vector3 translation_from_matrix(const Matrix34& mat)
    {
        return {
            mat.get(0, 3),
            mat.get(1, 3),
            mat.get(2, 3)
        };
    }
}