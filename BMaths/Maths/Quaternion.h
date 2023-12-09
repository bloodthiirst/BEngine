#pragma once
#include "Vector4.h"
#include "Vector3.h"
#include "Maths.h"
#include <math.h>
#include <memory>

struct Quaternion
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
            float i;
            float l;
            float j;
            float k;
        };

        struct
        {
            float values[4];
        };

        struct
        {
            Vector4 vec4;
        };
        struct
        {
            float scalar;
            Vector3 vector;
        };
    };

public:

    Quaternion () = default;

    Quaternion ( float x, float y, float z, float w )
    {
        memset(this, 0, sizeof(Quaternion));
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }

    static inline float Quadrant ( const Quaternion& src )
    {
        return (src.x * src.x) + (src.y * src.y) + (src.z * src.z) + (src.w * src.w);
    }
    static inline Quaternion Conjugate ( const Quaternion& src )
    {
        Quaternion res = src;
        res.vector.x = -res.vector.x;
        res.vector.y = -res.vector.y;
        res.vector.z = -res.vector.z;

        return res;
    }

    static inline Quaternion AxisRotation ( const float& angleInDegrees, const Vector3& axis )
    {
        // let's say we want to rotate a vector V by a quaternion Z , first we contruct the vector as a "pure quaternion" (means scalar part is 0) Q such as Q = (0 , V)
        // the Z * Q * Conjugate(Z) / Quadrant(Z) is use to rotate the vector with Z being the rotator
        // first , let's see the effect of Z * Q :
        // let's consider Z = ( cos(x/2) , sin(x/2) * u ) (the values will make sense once we continue)
        //// reminder :
        //// multiplying q1 by q2 
        //// q1 = ( x1 , v1 )
        //// q2 = ( x2 , v2 )
        //// q1 * q2 = ( x1       * x2 - dot(v1              , v2) , (x1       * v2) + (x2 * v1             ) + cross(v1              , v2 ) )
        // if we multiply Q and Z we will get
        //   Z * Q   = ( cos(x/2) * 0  - dot(sin(x/2) * axis , V ) , (cos(x/2) * V ) + (0  * sin(x/2) * axis) + cross(sin(x/2) * axis , V) )
        // we elimiate the mul by 0 stuff
        // Z * Q = -dot(sin(x/2) * axis , V) , (cos(x/2) * V) + cross(sin(x/2) * axis , V)
        // let's drop the scalar part
        // (cos(x/2) * V) + cross(sin(x/2) * axis , V)
        // cross(a , b) gives = length(a) * length(b) * sin(a^b) * N with N being a unit length vector perpedicular to both a and b
        // so cross (sin(x/2) * axis , V) = sin(x/2) * 1 * length(V) * sin(axis , V) * N
        // final result = Z * Q = (V * cos(x/2)) + ( N * length(V) * sin(axis , V) * sin(x/2))

        const float degToRad = Maths::DegreesToRadian ( angleInDegrees * 0.5f );
        float cosHalfAngle =  (float) cos ( degToRad );
        float sinHalfAngle = (float) sin ( degToRad );

        Quaternion res = {};
        res.scalar = cosHalfAngle;
        res.vector = axis * sinHalfAngle;

        return res;
    }

    /// <summary>
    /// <para>Q * Conjugate(Q) = Quadrant(Q) </para>
    /// <para>Quadrant(Q) is a number (scalar) , so we can divide both sides by Quadrant(Q) we get Q * Conjugate(Q) / Quadrant(Q) = 1</para>
    /// <para>If we say that (Conjugate(Q) / Quadrant(Q) ) = x , then we can replace it in the previous formla and say Q * x = 1</para>
    /// <para>Which is exacly what an invese is , so Inverse(Q) = Conjugate(Q) / Quadrant(Q) </para>
    /// <para>For unit quaternions, the inverse and the cojugate are the same </para>
    /// </summary>
    static inline Quaternion Inverse ( const Quaternion& src )
    {
        Quaternion q = Conjugate ( src );
        float quadrant = Quadrant ( src );
        q.x /= quadrant;
        q.y /= quadrant;
        q.z /= quadrant;
        q.w /= quadrant;
        return q;
    }

    inline friend Quaternion operator+( const Quaternion& lhs, const Quaternion& rhs )
    {
        Quaternion res = {};
        res.x = lhs.x + rhs.x;
        res.y = lhs.y + rhs.y;
        res.z = lhs.z + rhs.z;
        res.w = lhs.w + rhs.w;

        return res;
    }

    /// <summary>
    /// <para>Rotates the vector of the right by the quaternion on the left </para>
    /// </summary>
    /// <param name="lhs"></param>
    /// <param name="vec"></param>
    /// <returns></returns>
    inline friend Vector3 operator*( const Quaternion& lhs, const Vector3& vec )
    {
        Quaternion q = lhs;
        Quaternion qConj = Quaternion::Conjugate ( lhs );
        Quaternion vecAsQuat = Quaternion ( 0, vec.x, vec.y, vec.z );

        float quadrant = Quaternion::Quadrant ( q );

        Quaternion result = (q * vecAsQuat * qConj) / quadrant;

        return result.vector;
    }

    inline friend Quaternion operator*( const Quaternion& lhs, const float& rhs )
    {
        Quaternion res = lhs;
        res.x *= rhs;
        res.y *= rhs;
        res.z *= rhs;
        res.w *= rhs;
        return res;
    }

    inline friend Quaternion operator/( const Quaternion& lhs, const float& rhs )
    {
        Quaternion res = lhs;
        res.x /= rhs;
        res.y /= rhs;
        res.z /= rhs;
        res.w /= rhs;
        return res;
    }

    inline friend Quaternion operator*( const Quaternion& lhs, const Quaternion& rhs )
    {
        // with
        // q1 = ( x1 , v1 )
        // q2 = ( x2 , v2 )
        // q1 * q2 = ( x1 * x2 - dot(v1 , v2) , (x1 * v2) + (x2 * v1) + cross(v1 , v2) )
        Quaternion res = {};
        res.scalar = (lhs.scalar * rhs.scalar) - Vector3::Dot ( lhs.vector, rhs.vector );
        res.vector = (rhs.vector * lhs.scalar) + (lhs.vector * rhs.scalar) + Vector3::Cross ( lhs.vector, rhs.vector );
        return res;
    }

};