#pragma once
#include "Maths.h"
#include <memory>

struct Vector2
{
public:
    union
    {
        struct
        {
            float x;
            float y;
        };

        struct
        {
            float values[2];
        };
    };

public:
    Vector2() = default;
    Vector2( const float x, const float y)
    {
        memset( this, 0, sizeof( Vector2 ) );
        this->x = x;
        this->y = y;
    }
public:

    static Vector2 Up()
    {
        return Vector2( 0, 1 );
    }

    static Vector2 Down()
    {
        return Vector2( 0, -1 );
    }

    static Vector2 Right()
    {
        return Vector2( 1, 0 );
    }
    static Vector2 Left()
    {
        return Vector2( -1, 0 );
    }
    static Vector2 One()
    {
        return Vector2( 1, 1 );
    }
    static Vector2 Zero()
    {
        return Vector2( 0, 0 );
    }

public:

    static inline float MagnitudeSqr( const Vector2& vec )
    {
        float res = (vec.x * vec.x) + (vec.y * vec.y);

        return res;
    }

    static inline Vector2 Normalize( const Vector2& vec )
    {
        float mag = Magnitude( vec );

        return vec / mag;
    }

    static inline float Magnitude( const Vector2& vec )
    {
        return Maths::Sqrt( MagnitudeSqr( vec ) );
    }

public:
    friend inline Vector2 operator+( const Vector2& lhs, const Vector2& rhs )
    {
        Vector2 res = lhs;
        res.x += rhs.x;
        res.y += rhs.y;

        return res;
    }

    friend inline Vector2 operator-( const Vector2& lhs, const Vector2& rhs )
    {
        Vector2 res = lhs;
        res.x -= rhs.x;
        res.y -= rhs.y;

        return res;
    }

    friend inline Vector2 operator+( const Vector2& lhs, const float& rhs )
    {
        Vector2 res = lhs;
        res.x += rhs;
        res.y += rhs;

        return res;
    }

    friend inline Vector2 operator-( const Vector2& lhs, const float& rhs )
    {
        Vector2 res = lhs;
        res.x -= rhs;
        res.y -= rhs;


        return res;
    }

    friend inline Vector2 operator/( const Vector2& lhs, const float& rhs )
    {
        Vector2 res = lhs;
        res.x /= rhs;
        res.y /= rhs;

        return res;
    }


    friend inline Vector2 operator*( const Vector2& lhs, const float& rhs )
    {
        Vector2 res = lhs;
        res.x *= rhs;
        res.y *= rhs;

        return res;
    }

    static inline bool EqualsApprox( const Vector2& a, const Vector2& b )
    {
        return Vector2::MagnitudeSqr( a - b ) <= (Maths::Epsilon() * 2);
    }

    friend inline bool operator==( const Vector2& lhs, const Vector2& rhs )
    {
        return (lhs.x == rhs.x) && (lhs.y == rhs.y);
    }


    friend inline bool operator!=( const Vector2& lhs, const Vector2& rhs )
    {
        return !(lhs == rhs);
    }


    static inline float Dot( const Vector2& lhs, const Vector2& rhs )
    {
        float res =
            (lhs.x * rhs.x) +
            (lhs.y * rhs.y);

        return res;
    }
    /// <summary>
    /// <para>Return A cross B</para>
    /// <para>Notes:</para>
    /// <para>A x B = length(A) * length(B) * sin( A ^ B ) * N (with N being a normalized vector perpendicular to both A and B)</para>
    /// <para>The magnitude of the product equals the area of a parallelogram with the vectors for sides</para>
    /// <para>Using the right-hand rule with the thumb pointing up , if the index is A and the middle finger is B the thumb is A x B </para>
    /// <para>The magnitude of the product of two perpendicular vectors is the product of their lengths.</para>
    /// </summary>
    /// <param name="lhs"></param>
    /// <param name="rhs"></param>
    /// <returns></returns>
    static inline float Cross( const Vector2& lhs, const Vector2& rhs )
    {
        float res = (lhs.x * rhs.y - lhs.y * rhs.x);

        return res;
    }

};