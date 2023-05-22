#pragma once

namespace geom
{

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

}