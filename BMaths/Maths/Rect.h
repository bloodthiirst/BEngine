#pragma once
#include <Maths/Vector2.h>

struct Rect
{
    union
    {
        struct
        {
            float x;
            float y;
            float width;
            float height;
        };
        
        struct
        {
            Vector2 pos;
            Vector2 size;
        };
    };
};