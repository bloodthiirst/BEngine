#pragma once
#include <Containers/FreeList.h>
#include <Core/UI/UILayout.h>
#include <Core/Renderer/Font/Font.h>

struct TextCharData
{
    Matrix4x4 quad_matrix;
    Rect uv_rect;
};

struct TextUI
{
    FreeList::Node instance_matricies;
    StringView text;

    static TextUI Create(StringView text);
    static void Destroy(TextUI* txt);
    DrawMesh GetDraw();
};