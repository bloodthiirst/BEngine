#pragma once
#include "Vector4.h"
#include "Quaternion.h"
#include "Matrix3x3.h"
#include "Maths.h"
#include <memory>

struct Vector4Ref
{
public:
    float& x;
    float& y;
    float& z;
    float& w;

public:
    Vector4Ref ( float& x, float& y, float& z, float& w ) : x ( x ), y ( y ), z ( z ), w ( w )
    {}
};

struct Matrix4x4
{
public:
    union
    {
        struct
        {
            float r1c1;
            float r1c2;
            float r1c3;
            float r1c4;


            float r2c1;
            float r2c2;
            float r2c3;
            float r2c4;


            float r3c1;
            float r3c2;
            float r3c3;
            float r3c4;


            float r4c1;
            float r4c2;
            float r4c3;
            float r4c4;
        };

        struct
        {
            float values[16];
        };

        struct
        {
            Vector4 rows[4];
        };
    };

public:

    Matrix4x4 () = default;

    Matrix4x4 ( const Vector4 r1, const Vector4 r2, const Vector4 r3, const Vector4 r4 )
    {
        memset(this, 0, sizeof(Matrix4x4));
        rows[0] = r1;
        rows[1] = r2;
        rows[2] = r3;
        rows[3] = r4;
    }

public:

    inline float Get ( short column, short rows )
    {
        return values[column + (rows * 4)];
    }

    inline Vector4Ref GetRow ( int index )
    {
        short firstIndex = (index * 4);
        Vector4Ref vecRef ( values[firstIndex], values[firstIndex + 1], values[firstIndex + 2], values[firstIndex + 3] );
        return vecRef;
    }

    inline Vector4Ref GetColumn ( int index )
    {
        Vector4Ref vecRef ( values[index], values[index + 4], values[index + 8], values[index + 12] );
        return vecRef;
    }

public:
    static inline Matrix4x4 Identity ()
    {
        Matrix4x4 res = Matrix4x4 {};
        res.values[0] = 1;
        res.values[5] = 1;
        res.values[10] = 1;
        res.values[15] = 1;
        return res;
    }

    static inline Matrix4x4 Perspective ( const float& fov, float zNear, float zFar, const float& aspect )
    {

        float angle = Maths::DegreesToRadian ( fov );

        float w = (float) tan ( angle * 0.5f );
        float z = zFar - zNear;

        Matrix4x4 res = Matrix4x4 (
            { 1 / (aspect * w)  , 0     , 0                     , 0 },
            { 0                 , 1 / w , 0                     , 0 },
            { 0                 , 0     , -(zFar + zNear) / z   , -(2 * (zFar * zNear)) / z },
            { 0                 , 0     , -1                    , 0 }
        );

        return res;
    }

    static inline float Determinant ( Matrix4x4& source )
    {
        float det = 0;
        float mul;
        Matrix3x3 m3 = {};

        // 0
        {
            mul = source.values[0];
            m3.values[0] = source.Get ( 1, 1 ); m3.values[1] = source.Get ( 2, 1 ); m3.values[2] = source.Get ( 3, 1 );
            m3.values[3] = source.Get ( 1, 2 ); m3.values[4] = source.Get ( 2, 2 ); m3.values[5] = source.Get ( 3, 2 );
            m3.values[6] = source.Get ( 1, 3 ); m3.values[7] = source.Get ( 2, 3 ); m3.values[8] = source.Get ( 3, 3 );

            det += Matrix3x3::Determinant ( m3 ) * mul;
        }

        // 1
        {
            mul = source.values[1];
            m3.values[0] = source.Get ( 0, 1 ); m3.values[1] = source.Get ( 2, 1 ); m3.values[2] = source.Get ( 3, 1 );
            m3.values[3] = source.Get ( 0, 2 ); m3.values[4] = source.Get ( 2, 2 ); m3.values[5] = source.Get ( 3, 2 );
            m3.values[6] = source.Get ( 0, 3 ); m3.values[7] = source.Get ( 2, 3 ); m3.values[8] = source.Get ( 3, 3 );

            det += Matrix3x3::Determinant ( m3 ) * -mul;
        }

        // 2
        {
            mul = source.values[2];
            m3.values[0] = source.Get ( 0, 1 ); m3.values[1] = source.Get ( 1, 1 ); m3.values[2] = source.Get ( 3, 1 );
            m3.values[3] = source.Get ( 0, 2 ); m3.values[4] = source.Get ( 1, 2 ); m3.values[5] = source.Get ( 3, 2 );
            m3.values[6] = source.Get ( 0, 3 ); m3.values[7] = source.Get ( 1, 3 ); m3.values[8] = source.Get ( 3, 3 );

            det += Matrix3x3::Determinant ( m3 ) * mul;
        }

        // 3
        {
            mul = source.values[3];
            m3.values[0] = source.Get ( 0, 1 ); m3.values[1] = source.Get ( 1, 1 ); m3.values[2] = source.Get ( 2, 1 );
            m3.values[3] = source.Get ( 0, 2 ); m3.values[4] = source.Get ( 1, 2 ); m3.values[5] = source.Get ( 2, 2 );
            m3.values[6] = source.Get ( 0, 3 ); m3.values[7] = source.Get ( 1, 3 ); m3.values[8] = source.Get ( 2, 3 );

            det += Matrix3x3::Determinant ( m3 ) * -mul;
        }

        return det;

    }


    static inline bool ApproxEqual ( const Matrix4x4& lhs, const Matrix4x4& rhs, const float EPSILON )
    {
        bool r0 = Vector4::ApproxEqual ( lhs.rows[0], rhs.rows[0], EPSILON );
        bool r1 = Vector4::ApproxEqual ( lhs.rows[1], rhs.rows[1], EPSILON );
        bool r2 = Vector4::ApproxEqual ( lhs.rows[2], rhs.rows[2], EPSILON );
        bool r3 = Vector4::ApproxEqual ( lhs.rows[3], rhs.rows[3], EPSILON );

        return r0 && r1 && r2 && r3;
    }

    static inline Matrix4x4 Scale ( Vector3 scale )
    {
        Matrix4x4 res = Matrix4x4 (
            { scale.x,0,0 ,0 },
            { 0,scale.y,0 ,0 },
            { 0,0,scale.z ,0 },
            { 0,0,0       ,1 }
        );

        return res;
    }

    static inline Matrix4x4 Translate ( Vector3 translation )
    {
        Matrix4x4 res = Matrix4x4 (
            { 1,0,0,translation.x },
            { 0,1,0,translation.y },
            { 0,0,1,translation.z },
            { 0,0,0,1 }
        );

        return res;
    }

    static inline Matrix4x4 Rotate ( Quaternion q )
    {
        Matrix4x4 res = Matrix4x4
        (
            {
                2 * ((q.x * q.x) + (q.y * q.y)) - 1 ,
                2 * ((q.y * q.z) - (q.x * q.w)) ,
                2 * ((q.y * q.w) + (q.x * q.z)),
                0
            },

            {
                2 * ((q.y * q.z) + (q.x * q.w)) ,
                2 * ((q.x * q.x) + (q.z * q.z)) - 1 ,
                2 * ((q.z * q.w) - (q.x * q.y)),
                0
            },

            {
                2 * ((q.y * q.w) - (q.x * q.z)) ,
                2 * ((q.z * q.w) + (q.x * q.y)) ,
                2 * ((q.x * q.x) + (q.w * q.w)) - 1,
                0
            },

            {
                0,0,0,0
            }
        );

        return res;
    }

    static inline Matrix4x4 Inverse ( Matrix4x4& source )
    {
        Matrix4x4 res = {};

        float det = Determinant ( source );

        float detInverse = 1 / det;

        Matrix4x4 cofactor = Cofactor ( source );

        Matrix4x4 transposedCofactor = Transpose ( cofactor );

        res = transposedCofactor / det;

        return res;
    }

    inline void SubMatrix ( int r, int c, Matrix3x3& result )
    {
        int colInsert = 0;
        int rowInsert = 0;

        for ( int y = 0; y < 4; ++y )
        {
            if ( y != r )
            {
                for ( int x = 0; x < 4; ++x )
                {
                    if ( x != c )
                    {
                        result.values[colInsert + (rowInsert * 3)] = Get ( x, y );
                        colInsert++;
                    }


                }

                colInsert = 0;
                rowInsert++;
            }
        }

    }

    inline Matrix3x3 SubMatrix ( int r, int c )
    {
        Matrix3x3 res = {};

        SubMatrix ( r, c, res );

        return res;
    }

    static inline Matrix4x4 Cofactor ( Matrix4x4& source )
    {
        Matrix4x4 res = {};
        Matrix3x3 temp = {};
        float sign = 1;
        for ( int r = 0; r < 4; ++r )
        {
            for ( int c = 0; c < 4; c++ )
            {
                source.SubMatrix ( r, c, temp );
                res.values[c + (r * 4)] = Matrix3x3::Determinant ( temp ) * sign;
                sign *= -1;
            }

            sign *= -1;
        }

        return res;
    }

    static inline Matrix4x4 Transpose ( Matrix4x4& source )
    {
        Matrix4x4 res = {};

        for ( short i = 0; i < 4; ++i )
        {
            Vector4Ref vecRef = source.GetColumn ( i );
            res.rows[i].x = vecRef.x;
            res.rows[i].y = vecRef.y;
            res.rows[i].z = vecRef.z;
            res.rows[i].w = vecRef.w;
        }

        return res;
    }
public:

    inline Matrix4x4 operator/( float& div )
    {
        Matrix4x4 result = {};

        for ( unsigned short i = 0; i < 16; ++i )
        {
            result.values[i] = values[i] / div;
        }

        return result;
    }

    inline friend Vector3 operator*( const Matrix4x4& lhs, const Vector3& rhs )
    {
        Vector3 res = Vector3
        (
            (lhs.rows[0].x * rhs.x) + (lhs.rows[0].y * rhs.y) + (lhs.rows[0].z * rhs.z) + (lhs.rows[0].w),
            (lhs.rows[1].x * rhs.x) + (lhs.rows[1].y * rhs.y) + (lhs.rows[1].z * rhs.z) + (lhs.rows[1].w),
            (lhs.rows[2].x * rhs.x) + (lhs.rows[2].y * rhs.y) + (lhs.rows[2].z * rhs.z) + (lhs.rows[2].w)
        );

        return res;
    }

    inline Matrix4x4 operator*( float& mul )
    {
        Matrix4x4 result = {};

        for ( unsigned short i = 0; i < 16; ++i )
        {
            result.values[i] = values[i] * mul;
        }

        return result;
    }

    inline friend bool operator==( const Matrix4x4& lhs, const Matrix4x4& rhs )
    {
        return memcmp ( lhs.values, rhs.values, sizeof ( rhs.values ) ) == 0;
    }

    inline Matrix4x4 operator*( Matrix4x4 other )
    {
        // should be this.row * other.column
        Matrix4x4 result = {};

        for ( unsigned short c = 0; c < 4; ++c )
        {
            Vector4Ref col = other.GetColumn( c );

            for ( unsigned short r = 0; r < 4; ++r )
            {
                Vector4 r_row = rows[r];

                result.values[c + (r * 4)] =
                    (r_row.x * col.x) +
                    (r_row.y * col.y) +
                    (r_row.z * col.z) +
                    (r_row.w * col.w);
            }
        }

        return result;
    }

};