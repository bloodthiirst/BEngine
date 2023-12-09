#pragma once
#include "Maths.h"
#include <memory>

struct Vector3
{
public:
    union
    {
        struct
        {
            float x;
            float y;
            float z;
        };

        struct
        {
            float values[3];
        };
    };

public:
    Vector3 () = default;
    Vector3 (const float x, const float y,const  float z )
    {
        memset(this, 0, sizeof(Vector3));
        this->x = x;
        this->y = y;
        this->z = z;
    }
public:

    static Vector3 Up ()
    {
        return Vector3 ( 0, 1, 0);
    }

    static Vector3 Down ()
    {
        return Vector3 ( 0, -1, 0 );
    }

    static Vector3 Right ()
    {
        return Vector3 ( 1, 0, 0 );
    }
    static Vector3 Left ()
    {
        return Vector3 ( -1, 0, 0 );
    }
    static Vector3 Forward ()
    {
        return Vector3 ( 0, 0, 1 );
    }
    static Vector3 Backward ()
    {
        return Vector3 ( 0, 0, -1 );
    }
    static Vector3 One ()
    {
        return Vector3 ( 1, 1, 1 );
    }
    static Vector3 Zero ()
    {
        return Vector3 ( 0, 0, 0 );
    }

public:

    static inline float MagnitudeSqr ( const Vector3& vec )
    {
        float res = (vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.y);

        return res;
    }

    static inline Vector3 Normalize ( const Vector3& vec )
    {
        float mag = Magnitude ( vec );

        return vec / mag;
    }

    static inline float Magnitude (const Vector3& vec )
    {
        return Maths::Sqrt ( MagnitudeSqr ( vec ) );
    }

public:
    friend inline Vector3 operator+( const Vector3& lhs, const Vector3& rhs )
    {
        Vector3 res = lhs;
        res.x += rhs.x;
        res.y += rhs.y;
        res.z += rhs.z;

        return res;
    }

    friend inline Vector3 operator-( const Vector3& lhs, const Vector3& rhs )
    {
        Vector3 res = lhs;
        res.x -= rhs.x;
        res.y -= rhs.y;
        res.z -= rhs.z;

        return res;
    }

    friend inline Vector3 operator+( const Vector3& lhs, const float& rhs )
    {
        Vector3 res = lhs;
        res.x += rhs;
        res.y += rhs;
        res.z += rhs;

        return res;
    }

    friend inline Vector3 operator-( const Vector3& lhs, const float& rhs)
    {
        Vector3 res = lhs;
        res.x -= rhs;
        res.y -= rhs;
        res.z -= rhs;

        return res;
    }

    friend inline Vector3 operator/( const Vector3& lhs, const float& rhs )
    {
        Vector3 res = lhs;
        res.x /= rhs;
        res.y /= rhs;
        res.z /= rhs;

        return res;
    }


    friend inline Vector3 operator*( const Vector3& lhs , const float& rhs)
    {
        Vector3 res = lhs;
        res.x *= rhs;
        res.y *= rhs;
        res.z *= rhs;

        return res;
    }

    static inline bool EqualsApprox ( const Vector3& a, const Vector3& b )
    {
        return Vector3::MagnitudeSqr ( a - b ) <= (Maths::Epsilon() * 3);
    }

    friend inline bool operator==( const Vector3& lhs, const Vector3& rhs )
    {
        return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z);
    }


    friend inline bool operator!=( const Vector3& lhs, const Vector3& rhs )
    {
        return !(lhs == rhs);
    }


    static inline float Dot ( const Vector3& lhs, const Vector3& rhs )
    {
        float res =
            (lhs.x * rhs.x) +
            (lhs.y * rhs.y) +
            (lhs.z * rhs.z);

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
    static inline Vector3 Cross ( const Vector3& lhs, const Vector3& rhs )
    {
        Vector3 res = { };

        res.x = (lhs.y * rhs.z - lhs.z * rhs.y);
        res.y = (lhs.z * rhs.x - lhs.x * rhs.z);
        res.z = (lhs.x * rhs.y - lhs.y * rhs.x);

        return res;
    }

};