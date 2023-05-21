#include "vector3.h"
#include "matrix.h"

//this file handles combined functionality between vectors, matrices, quaternions

namespace geom
{
    //operations

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

    Matrix34 create_translation_matrix(Vector3 vec)
    {
        auto result = Matrix34::identity();
        result.get(0, 3) = vec.x;
        result.get(1, 3) = vec.y;
        result.get(2, 3) = vec.z;
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