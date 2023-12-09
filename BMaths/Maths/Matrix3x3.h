#pragma once
#include "Vector3.h"

struct Vector3Ref
{
public:
    float& x;
    float& y;
    float& z;

public:
    Vector3Ref ( float& x, float& y, float& z) : x ( x ), y ( y ), z ( z )
    {}
};

struct Matrix3x3
{
public:
    union
    {
        struct
        {
            float r1c1;
            float r1c2;
            float r1c3;


            float r2c1;
            float r2c2;
            float r2c3;


            float r3c1;
            float r3c2;
            float r3c3;
        };

        struct
        {
            float values[9];
        };

        struct
        {
            Vector3 rows[3];
        };
    };

public:
    inline Vector3Ref GetRow ( int index )
    {
        Vector3Ref vecRef ( values[index], values[index + 1], values[index + 2]);
        return vecRef;
    }

    inline Vector3Ref GetColumn ( int index )
    {
        Vector3Ref vecRef ( values[index], values[3 + index], values[6 + index]);
        return vecRef;
    }

public:

    inline float Get ( short x, short y )
    {
        return values[x + (y * 3)];
    }

    inline static float Determinant ( Matrix3x3& source )
    {
        float det = 0;

        float a = source.values[0] *
            (
                (source.Get ( 1, 1 ) * source.Get ( 2, 2 )) -
                (source.Get ( 1, 2 ) * source.Get ( 2, 1 ))
            );

        float b = -source.values[1] *
            (
                (source.Get ( 0, 1 ) * source.Get ( 2, 2 )) -
                (source.Get ( 0, 2 ) * source.Get ( 2, 1 ))
            );

        float c = source.values[2] *
            (
                (source.Get ( 0, 1 ) * source.Get ( 1, 2 )) -
                (source.Get ( 0, 2 ) * source.Get ( 1, 1 ))
            );

        det = a + b + c;

        return det;
    }

    inline static Matrix3x3 Transpose ( Matrix3x3& source )
    {
        Matrix3x3 res = {};

        for ( short i = 0; i < 3; ++i )
        {
            Vector3Ref vecRef = source.GetColumn ( i );
            res.rows->x = vecRef.x;
            res.rows->y = vecRef.y;
            res.rows->z = vecRef.z;
        }

        return res;
    }
public:

    Matrix3x3 operator*( Matrix3x3& other )
    {
        Matrix3x3 result = {};

        for ( unsigned short i = 0; i < 4; ++i )
        {
            Vector3Ref col = other.GetColumn ( i );

            for ( unsigned short i = 0; i < 4; ++i )
            {
                Vector3 row = rows[0];

                result.values[i] =
                    row.x * col.x +
                    row.y * col.y +
                    row.z * col.z;
            }
        }

        return result;
    }

};