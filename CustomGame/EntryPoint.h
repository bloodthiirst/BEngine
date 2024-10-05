#pragma once
#include <Containers/FreeList.h>
#include <Core/Renderer/Mesh/Mesh3D.h>
#include <Core/Renderer/Texture/Texture.h>
#include <Core/Renderer/Buffer/Buffer.h>
#include <Core/Renderer/Font/Font.h>
#include <Core/UI/UILayout.h>
#include <Core/Thread/Thread.h>
#include "TextUI.h"
#include "SceneCameraController.h"

struct EntryPoint
{
    SceneCameraController camera_controller;
    Mesh3D plane_mesh;
    ShaderBuilder ui_shader_builder;
    ShaderBuilder text_shader_builder;
    Texture ui_texture;
    FontInfo font_info;
    RootLayoutNode ui_root;
    Thread thread_test;
    TextUI text;
};