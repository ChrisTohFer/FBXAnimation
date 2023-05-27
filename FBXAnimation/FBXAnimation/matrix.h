#pragma once

namespace geom
{
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

    //operators
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

    //specialisation
    using Matrix44 = Matrix<4, 4>;
    using Matrix34 = Matrix<3, 4>;
    
    /*
    struct Matrix44
    {
        float values[16];

        static Matrix44 identity();

        int index(int row, int col) const;
        float get(int row, int col) const;
        float& get(int row, int col);
    };

    struct Matrix34
    {
        float values[12];

        static Matrix34 identity();

        int index(int row, int col) const;
        float get(int row, int col) const;
        float& get(int row, int col);
    };

    //inline non-member functions

    Matrix44 operator*(const Matrix44& lhs, const Matrix44& rhs)
    {
        Matrix44 result;

        //this function could probably be more concise...

        //column 0
        result.get(0, 0) =
            lhs.get(0, 0) * rhs.get(0, 0) +
            lhs.get(0, 1) * rhs.get(1, 0) +
            lhs.get(0, 2) * rhs.get(2, 0) +
            lhs.get(0, 3) * rhs.get(3, 0);
        result.get(1, 0) =
            lhs.get(1, 0) * rhs.get(0, 0) +
            lhs.get(1, 1) * rhs.get(1, 0) +
            lhs.get(1, 2) * rhs.get(2, 0) +
            lhs.get(1, 3) * rhs.get(3, 0);
        result.get(2, 0) =
            lhs.get(2, 0) * rhs.get(0, 0) +
            lhs.get(2, 1) * rhs.get(1, 0) +
            lhs.get(2, 2) * rhs.get(2, 0) +
            lhs.get(2, 3) * rhs.get(3, 0);
        result.get(3, 0) =
            lhs.get(3, 0) * rhs.get(0, 0) +
            lhs.get(3, 1) * rhs.get(1, 0) +
            lhs.get(3, 2) * rhs.get(2, 0) +
            lhs.get(3, 3) * rhs.get(3, 0);

        //column 1
        result.get(0, 1) =
            lhs.get(0, 0) * rhs.get(0, 1) +
            lhs.get(0, 1) * rhs.get(1, 1) +
            lhs.get(0, 2) * rhs.get(2, 1) +
            lhs.get(0, 3) * rhs.get(3, 1);
        result.get(1, 1) =
            lhs.get(1, 0) * rhs.get(0, 1) +
            lhs.get(1, 1) * rhs.get(1, 1) +
            lhs.get(1, 2) * rhs.get(2, 1) +
            lhs.get(1, 3) * rhs.get(3, 1);
        result.get(2, 1) =
            lhs.get(2, 0) * rhs.get(0, 1) +
            lhs.get(2, 1) * rhs.get(1, 1) +
            lhs.get(2, 2) * rhs.get(2, 1) +
            lhs.get(2, 3) * rhs.get(3, 1);
        result.get(3, 1) =
            lhs.get(3, 0) * rhs.get(0, 1) +
            lhs.get(3, 1) * rhs.get(1, 1) +
            lhs.get(3, 2) * rhs.get(2, 1) +
            lhs.get(3, 3) * rhs.get(3, 1);

        //column 2
        result.get(0, 2) =
            lhs.get(0, 0) * rhs.get(0, 2) +
            lhs.get(0, 1) * rhs.get(1, 2) +
            lhs.get(0, 2) * rhs.get(2, 2) +
            lhs.get(0, 3) * rhs.get(3, 2);
        result.get(1, 2) =
            lhs.get(1, 0) * rhs.get(0, 2) +
            lhs.get(1, 1) * rhs.get(1, 2) +
            lhs.get(1, 2) * rhs.get(2, 2) +
            lhs.get(1, 3) * rhs.get(3, 2);
        result.get(2, 2) =
            lhs.get(2, 0) * rhs.get(0, 2) +
            lhs.get(2, 1) * rhs.get(1, 2) +
            lhs.get(2, 2) * rhs.get(2, 2) +
            lhs.get(2, 3) * rhs.get(3, 2);
        result.get(3, 2) =
            lhs.get(3, 0) * rhs.get(0, 2) +
            lhs.get(3, 1) * rhs.get(1, 2) +
            lhs.get(3, 2) * rhs.get(2, 2) +
            lhs.get(3, 3) * rhs.get(3, 2);

        //column 3
        result.get(0, 3) =
            lhs.get(0, 0) * rhs.get(0, 3) +
            lhs.get(0, 1) * rhs.get(1, 3) +
            lhs.get(0, 2) * rhs.get(2, 3) +
            lhs.get(0, 3) * rhs.get(3, 3);
        result.get(1, 3) =
            lhs.get(1, 0) * rhs.get(0, 3) +
            lhs.get(1, 1) * rhs.get(1, 3) +
            lhs.get(1, 2) * rhs.get(2, 3) +
            lhs.get(1, 3) * rhs.get(3, 3);
        result.get(2, 3) =
            lhs.get(2, 0) * rhs.get(0, 3) +
            lhs.get(2, 1) * rhs.get(1, 3) +
            lhs.get(2, 2) * rhs.get(2, 3) +
            lhs.get(2, 3) * rhs.get(3, 3);
        result.get(3, 3) =
            lhs.get(3, 0) * rhs.get(0, 3) +
            lhs.get(3, 1) * rhs.get(1, 3) +
            lhs.get(3, 2) * rhs.get(2, 3) +
            lhs.get(3, 3) * rhs.get(3, 3);

        return result;
    }

    Matrix34 operator*(const Matrix34& lhs, const Matrix34& rhs)
    {
        Matrix34 result;

        //this function could probably be more concise...

        //column 0
        result.get(0, 0) =
            lhs.get(0, 0) * rhs.get(0, 0) +
            lhs.get(0, 1) * rhs.get(1, 0) +
            lhs.get(0, 2) * rhs.get(2, 0);
        result.get(1, 0) =
            lhs.get(1, 0) * rhs.get(0, 0) +
            lhs.get(1, 1) * rhs.get(1, 0) +
            lhs.get(1, 2) * rhs.get(2, 0);
        result.get(2, 0) =
            lhs.get(2, 0) * rhs.get(0, 0) +
            lhs.get(2, 1) * rhs.get(1, 0) +
            lhs.get(2, 2) * rhs.get(2, 0);

        //column 1
        result.get(0, 1) =
            lhs.get(0, 0) * rhs.get(0, 1) +
            lhs.get(0, 1) * rhs.get(1, 1) +
            lhs.get(0, 2) * rhs.get(2, 1);
        result.get(1, 1) =
            lhs.get(1, 0) * rhs.get(0, 1) +
            lhs.get(1, 1) * rhs.get(1, 1) +
            lhs.get(1, 2) * rhs.get(2, 1);
        result.get(2, 1) =
            lhs.get(2, 0) * rhs.get(0, 1) +
            lhs.get(2, 1) * rhs.get(1, 1) +
            lhs.get(2, 2) * rhs.get(2, 1);

        //column 2
        result.get(0, 2) =
            lhs.get(0, 0) * rhs.get(0, 2) +
            lhs.get(0, 1) * rhs.get(1, 2) +
            lhs.get(0, 2) * rhs.get(2, 2);
        result.get(1, 2) =
            lhs.get(1, 0) * rhs.get(0, 2) +
            lhs.get(1, 1) * rhs.get(1, 2) +
            lhs.get(1, 2) * rhs.get(2, 2);
        result.get(2, 2) =
            lhs.get(2, 0) * rhs.get(0, 2) +
            lhs.get(2, 1) * rhs.get(1, 2) +
            lhs.get(2, 2) * rhs.get(2, 2);

        //column 3
        result.get(0, 3) =
            lhs.get(0, 0) * rhs.get(0, 3) +
            lhs.get(0, 1) * rhs.get(1, 3) +
            lhs.get(0, 2) * rhs.get(2, 3) +
            lhs.get(0, 3);
        result.get(1, 3) =
            lhs.get(1, 0) * rhs.get(0, 3) +
            lhs.get(1, 1) * rhs.get(1, 3) +
            lhs.get(1, 2) * rhs.get(2, 3) +
            lhs.get(1, 3);
        result.get(2, 3) =
            lhs.get(2, 0) * rhs.get(0, 3) +
            lhs.get(2, 1) * rhs.get(1, 3) +
            lhs.get(2, 2) * rhs.get(2, 3) +
            lhs.get(2, 3);

        return result;
    }

    //inline member functions

    //Matrix44

    Matrix44 Matrix44::identity()
    {
        Matrix44 result;

        result.get(0, 0) = 1.f;
        result.get(1, 0) = 0.f;
        result.get(2, 0) = 0.f;
        result.get(3, 0) = 0.f;

        result.get(0, 1) = 0.f;
        result.get(1, 1) = 1.f;
        result.get(2, 1) = 0.f;
        result.get(3, 1) = 0.f;

        result.get(0, 2) = 0.f;
        result.get(1, 2) = 0.f;
        result.get(2, 2) = 1.f;
        result.get(3, 2) = 0.f;

        result.get(0, 3) = 0.f;
        result.get(1, 3) = 0.f;
        result.get(2, 3) = 0.f;
        result.get(3, 3) = 1.f;

        return result;
    }

    int Matrix44::index(int row, int col) const
    {
        return row + col * 4;
    }

    float Matrix44::get(int row, int col) const
    {
        return values[index(row, col)];
    }

    float& Matrix44::get(int row, int col)
    {
        return values[index(row, col)];
    }
    
    //Matrix34

    Matrix34 Matrix34::identity()
    {
        Matrix34 result;

        result.get(0, 0) = 1.f;
        result.get(1, 0) = 0.f;
        result.get(2, 0) = 0.f;

        result.get(0, 1) = 0.f;
        result.get(1, 1) = 1.f;
        result.get(2, 1) = 0.f;

        result.get(0, 2) = 0.f;
        result.get(1, 2) = 0.f;
        result.get(2, 2) = 1.f;

        result.get(0, 3) = 0.f;
        result.get(1, 3) = 0.f;
        result.get(2, 3) = 0.f;

        return result;
    }

    int Matrix34::index(int row, int col) const
    {
        return row + col * 3;
    }

    float Matrix34::get(int row, int col) const
    {
        return values[index(row, col)];
    }

    float& Matrix34::get(int row, int col)
    {
        return values[index(row, col)];
    }
    */
}