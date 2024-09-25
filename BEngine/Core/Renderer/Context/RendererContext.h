#pragma once
#include <Containers/DArray.h>
#include "../Shader/Shader.h"
#include "../Mesh/Mesh3D.h"

struct DrawMesh
{
    Mesh3D* mesh;
    ShaderBuilder* shader_builder;
    Texture* texture;
};

struct RendererContext
{
    DArray<DrawMesh> mesh_draws;
};