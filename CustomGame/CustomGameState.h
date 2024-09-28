#pragma once
#include <Core/Renderer/Mesh/Mesh3D.h>
#include <Core/Renderer/Texture/Texture.h>
#include <Core/UI/UILayout.h>
#include "SceneCameraController.h"

struct CustomGameState
{
    SceneCameraController camera_controller;
    Mesh3D plane_mesh;
    ShaderBuilder shader_builder;
    Texture texture;
    LayoutNode ui_root;
};