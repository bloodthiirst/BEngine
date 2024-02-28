#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include <cfloat>

class Maths
{
public:

    static inline float Epsilon()
    {
        return FLT_EPSILON;
    }

    static inline float Sqrt(float value)
    {
        return (float) sqrt(value);
    }

    template<typename T>
    static inline T DegreesToRadian ( const T& degrees )
    {
        static const T mul = (float) M_PI / 180.0f;

        return degrees * mul;
    }

    template<typename T>
    static inline T RadiansToDegrees ( const T& radians )
    {
        static const T mul = 180.0f / M_PI ;

        return radians * mul;
    }

    static float ClampAngle( float angle_in_degrees )
    {
        float sign = angle_in_degrees < 0 ? -1.0f : 1.0;
        int full_2pi = ((int) (abs( (long) angle_in_degrees ) / 360.0));

        if ( angle_in_degrees < 0 )
        {
            full_2pi++;
        }

        float clamped_angle = (angle_in_degrees + (full_2pi * 360 * -sign));

        return clamped_angle;
    }


	template<typename T>
	static T Clamp ( T value, T min, T max )
	{
		if ( value <= min )
			return min;

		if ( value >= max )
			return max;

		return  value;
	}


};