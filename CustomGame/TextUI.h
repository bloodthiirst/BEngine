#pragma once
#include <Containers/FreeList.h>
#include <Core/UI/UILayout.h>
#include <Core/Renderer/Font/Font.h>

struct TextCharData
{
    Rect uv;
    Matrix4x4 mat;
};

struct TextUI
{
    FreeList::Node instance_matricies;
    StringView text;

    static TextUI Create(StringView text);
    static void Destroy(TextUI* txt);
    DrawMesh GetDraw();
};