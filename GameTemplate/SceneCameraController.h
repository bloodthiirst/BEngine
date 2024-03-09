#pragma once
#include <Maths/Vector3.h>
#include <Maths/Quaternion.h>

struct SceneCameraController
{
    float xRotation;
    float yRotation;

    Vector3 position;

    Vector3 mousePos;

    // roll (x), pitch (y), yaw (z), angles are in radians
    Quaternion ToQuaternion( double roll, double pitch, double yaw ) 
    {
        // Abbreviations for the various angular functions

        double cr = cos( roll * 0.5 );
        double sr = sin( roll * 0.5 );
        double cp = cos( pitch * 0.5 );
        double sp = sin( pitch * 0.5 );
        double cy = cos( yaw * 0.5 );
        double sy = sin( yaw * 0.5 );

        Quaternion q;
        q.z = cr * cp * cy + sr * sp * sy;
        q.w = sr * cp * cy - cr * sp * sy;
        q.x = cr * sp * cy + sr * cp * sy;
        q.y = cr * cp * sy - sr * sp * cy;

        return q;
    }

    void Tick( float delta_time, Vector3* out_pos, Quaternion* out_rot )
    {
        float rotation_speed = 30;
        Vector3 move_delta = { 0,0,0 };
        Vector3 rotation_delta = { 0,0,0 };

        // movement
        {
            {
                move_delta.z += Global::platform.input.mouseState.currentScrollWheelDelta;
            }

            if ( Global::platform.input.IsPressed( KeyCode::KEY_RIGHT ) )
            {
                move_delta.x += 1;
            }

            if ( Global::platform.input.IsPressed( KeyCode::KEY_LEFT ) )
            {
                move_delta.x -= 1;
            }

            if ( Global::platform.input.IsPressed( KeyCode::KEY_UP ) )
            {
                move_delta.y += 1;
            }

            if ( Global::platform.input.IsPressed( KeyCode::KEY_DOWN ) )
            {
                move_delta.y -= 1;
            }
        }

        // rotation
        {
            if ( Global::platform.input.IsPressed( KeyCode::KEY_Z ) )
            {
                rotation_delta.y -= 1;
            }

            if ( Global::platform.input.IsPressed( KeyCode::KEY_S ) )
            {
                rotation_delta.y += 1;
            }

            if ( Global::platform.input.IsPressed( KeyCode::KEY_D ) )
            {
                rotation_delta.x += 1;
            }

            if ( Global::platform.input.IsPressed( KeyCode::KEY_Q ) )
            {
                rotation_delta.x -= 1;
            }
        }

        if ( rotation_delta != Vector3::Zero() )
        {
            rotation_delta = Vector3::Normalize( rotation_delta );

            xRotation = xRotation + (rotation_delta.x * delta_time * rotation_speed);
            yRotation = yRotation + (rotation_delta.y * delta_time * rotation_speed);

            // xRotation = Maths::ClampAngle( xRotation );
            // yRotation = Maths::ClampAngle( yRotation );
        }

        Quaternion pitch_rot = Quaternion::AxisRotation( yRotation, Vector3::Right() );
        Quaternion yaw_rot = Quaternion::AxisRotation( xRotation, Vector3::Up() );
        Quaternion total_rot = yaw_rot * (yaw_rot * pitch_rot);

        if ( move_delta != Vector3::Zero() )
        {
            move_delta = Vector3::Normalize( move_delta );
            position = position + ((total_rot * move_delta) * delta_time);
        }

        *out_rot = total_rot;
        *out_pos = position;
    }
};