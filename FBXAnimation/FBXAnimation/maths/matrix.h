#pragma once

namespace geom
{
    //forward declarations

    struct Quaternion;
    struct Vector3;

    //struct

    template<int Rows, int Columns>
    struct Matrix
    {
        float values[Rows * Columns];

        static Matrix identity();

        int index(int row, int col) const;
        float get(int row, int col) const;
        float& get(int row, int col);

        float determinant() const requires(Rows == Columns && Rows != 1);
        float determinant() const;
        Matrix<Rows - 1, Columns - 1> submatrix(int row, int col) const;
        Matrix adjugate() const;
        Matrix inverse() const;
    };

    //operators

    template<int Rows, int Columns>
    Matrix<Rows, Columns> operator*(const Matrix<Rows, Columns>& lhs, const Matrix<Rows, Columns>& rhs);
    template<int Rows, int Columns>
    Matrix<Rows, Columns> operator*(const Matrix<Rows, Columns>& lhs, float rhs);
    template<int Rows, int Columns>
    Matrix<Rows, Columns> operator*(float lhs, const Matrix<Rows, Columns>& rhs);
    template<int Rows, int Columns>
    Matrix<Rows, Columns> operator/(const Matrix<Rows, Columns>& lhs, float rhs);

    //specialisation
    using Matrix44 = Matrix<4, 4>;
    using Matrix34 = Matrix<3, 4>;
}

//deliberately included after declarations to prevent circular dependency
#include "quaternion.h"
#include "vector3.h"

//inline definitions
namespace geom
{
    //inline operator definitions

    template<int Rows, int Columns>
    Matrix<Rows, Columns> operator*(const Matrix<Rows, Columns>& lhs, const Matrix<Rows, Columns>& rhs)
    {
        Matrix<Rows, Columns> result;

        for (int column = 0; column < Columns; ++column)
        {
            for (int row = 0; row < Rows; ++row)
            {
                result.get(row, column) =
                    lhs.get(row, 0) * rhs.get(0, column) +
                    lhs.get(row, 1) * rhs.get(1, column) +
                    lhs.get(row, 2) * rhs.get(2, column) +
                    lhs.get(row, 3) * rhs.get(3, column);
            }
        }

        return result;
    }
    template<int Rows, int Columns>
    Matrix<Rows, Columns> operator*(const Matrix<Rows, Columns>& lhs, float rhs)
    {
        Matrix<Rows, Columns> matrix;
        for (int i = 0; i < Rows * Columns; ++i)
        {
            matrix.values[i] = lhs.values[i] * rhs;
        }
        return matrix;
    }
    template<int Rows, int Columns>
    Matrix<Rows, Columns> operator*(float lhs, const Matrix<Rows, Columns>& rhs)
    {
        return rhs * lhs;
    }
    template<int Rows, int Columns>
    Matrix<Rows, Columns> operator/(const Matrix<Rows, Columns>& lhs, float rhs)
    {
        Matrix<Rows, Columns> matrix;
        for (int i = 0; i < Rows * Columns; ++i)
        {
            matrix.values[i] = lhs.values[i] / rhs;
        }
        return matrix;
    }

    //inline member definitions
    template<int Rows, int Columns>
    Matrix<Rows, Columns> Matrix<Rows, Columns>::identity()
    {
        Matrix mat;
        for (int column = 0; column < Columns; ++column)
        {
            for (int row = 0; row < Rows; ++row)
            {
                mat.get(row, column) = (float)(row == column);
            }
        }

        return mat;
    }

    template<int Rows, int Columns>
    int Matrix<Rows, Columns>::index(int row, int col) const
    {
        return row + col * Rows;
    }

    template<int Rows, int Columns>
    float Matrix<Rows, Columns>::get(int row, int col) const
    {
        return values[index(row, col)];
    }

    template<int Rows, int Columns>
    float& Matrix<Rows, Columns>::get(int row, int col)
    {
        return values[index(row, col)];
    }

    template<int Rows, int Columns>
    float Matrix<Rows, Columns>::determinant() const requires(Rows == Columns && Rows != 1)
    {
        float result = 0;
        for (int row = 0; row < Rows; ++row)
        {
            float sign = (row % 2 == 0) ? 1.f : -1.f;
            result += sign * get(row, 0) * submatrix(row, 0).determinant();
        }
        return result;
    }

    template<>
    float Matrix<1, 1>::determinant() const
    {
        return values[0];
    }

    template<int Rows, int Columns>
    Matrix<Rows - 1, Columns - 1> Matrix<Rows, Columns>::submatrix(int i, int j) const
    {
        Matrix<Rows - 1, Columns - 1> result;
        for (int column = 0; column < Columns - 1; ++column)
        {
            int from_column = column < j ? column : column + 1;
            for (int row = 0; row < Rows - 1; ++row)
            {
                int from_row = row < i ? row : row + 1;

                result.get(row, column) = get(from_row, from_column);
            }
        }

        return result;
    }

    template<int Rows, int Columns>
    Matrix<Rows, Columns> Matrix<Rows, Columns>::adjugate() const
    {
        Matrix result;

        for (int column = 0; column < Columns; ++column)
        {
            for (int row = 0; row < Rows; ++row)
            {
                float sign = ((row + column) % 2 == 0) ? 1.f : -1.f;
                result.get(row, column) = sign * submatrix(row, column).determinant();
            }
        }

        return result;
    }

    template<int Rows, int Columns>
    Matrix<Rows, Columns> Matrix<Rows, Columns>::inverse() const
    {
        return adjugate() / determinant();
    }
}