#pragma once
#include "../../Platform/Base/Platform.h"
#include "../../Application/Application.h"
#include "../Context/RendererContext.h"
#include <Maths/Matrix4x4.h>
#include <Maths/Vector3.h>

struct BackendRenderer
{
    size_t frame_count = { 0 };

    void* user_data;

    Func<bool,BackendRenderer* , ApplicationStartup> startup;

    ActionParams<BackendRenderer*, uint32_t, uint32_t> resize;
    Func<bool, BackendRenderer*, RendererContext> start_frame;
    Func<bool, BackendRenderer*, Matrix4x4, Matrix4x4, float, uint32_t > update_global_state;
    Func<bool, BackendRenderer*, RendererContext> end_frame;

    Func<bool, BackendRenderer*> destroy;
};

