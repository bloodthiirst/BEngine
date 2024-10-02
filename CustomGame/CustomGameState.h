#pragma once
#include <Core/Renderer/Mesh/Mesh3D.h>
#include <Core/Renderer/Texture/Texture.h>
#include <Core/Renderer/Buffer/Buffer.h>
#include <Core/UI/UILayout.h>
#include <Containers/FreeList.h>
#include <Core/Thread/Thread.h>
#include "SceneCameraController.h"

struct CustomGameState
{
    SceneCameraController camera_controller;
    Mesh3D plane_mesh;
    ShaderBuilder shader_builder;
    Texture texture;
    FreeList::Node instances_data;
    LayoutNode ui_root;
    Thread thread_test;
};