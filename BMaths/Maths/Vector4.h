#pragma once
#include <math.h>
#include <memory>

struct Vector4
{
public:
    union
    {
        struct
        {
            float x;
            float y;
            float z;
            float w;
        };

        struct
        {
            float values[4];
        };
    };

public:

    Vector4 () = default;

    Vector4 ( float x, float y, float z, float w )
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }

public:

    static inline float Dot ( Vector4& lhs, Vector4& rhs )
    {
        float res = (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z) + (lhs.w * rhs.w);

        return res;
    }

    static inline float MagnitudeSqr ( Vector4& vec )
    {
        float res = (vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.y) + (vec.w * vec.w);

        return res;
    }

    static inline Vector4 Normalize ( Vector4& vec )
    {
        float mag = Magnitude ( vec );

        return vec / mag;
    }

    static inline float Magnitude ( Vector4& vec )
    {
        return (float) sqrt ( MagnitudeSqr ( vec ) );
    }

    static inline bool ApproxEqual ( const Vector4& lhs, const Vector4& rhs , const float EPSILON )
    {
        Vector4 diff = rhs - lhs;

        return MagnitudeSqr ( diff ) < EPSILON;
    }

    Vector4 operator+( const Vector4& other ) const
    {
        Vector4 res = Vector4 ( x + other.x, y + other.y, z + other.z, w + other.w );

        return res;
    }

    Vector4 operator-( const Vector4& other ) const
    {
        Vector4 res = Vector4 ( x - other.x, y - other.y, z - other.z, w - other.w );

        return res;
    }

    Vector4 operator*( const float& mul ) const
    {
        Vector4 res = Vector4 ( x, y, z, w );
        res.x *= mul;
        res.y *= mul;
        res.z *= mul;
        res.w *= mul;

        return res;
    }

    Vector4 operator/( const float& mul )
    {
        float rcp = 1 / mul;
        Vector4 res = Vector4 ( *this );
        return res * rcp;
    }

    friend bool operator==( const Vector4& lhs, const Vector4& rhs )
    {
        return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
    }

    friend bool operator!=( const Vector4& lhs, const Vector4& rhs )
    {
        return !(lhs == rhs);
    }

};
