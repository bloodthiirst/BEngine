#pragma once
#include <Core/Renderer/Mesh/Mesh3D.h>
#include <Core/Renderer/Texture/Texture.h>
#include <Core/Renderer/Buffer/Buffer.h>
#include <Core/UI/UILayout.h>
#include "SceneCameraController.h"

struct CustomGameState
{
    SceneCameraController camera_controller;
    Mesh3D plane_mesh;
    ShaderBuilder shader_builder;
    Texture texture;
    Buffer instances_data;
    LayoutNode ui_root;
};