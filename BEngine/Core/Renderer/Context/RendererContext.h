#pragma once
#include <Containers/DArray.h>
#include "../Shader/Shader.h"
#include "../Mesh/Mesh3D.h"
#include <Containers/FreeList.h>

struct DrawMesh
{
    Mesh3D* mesh;
    ShaderBuilder* shader_builder;
    Texture* texture;
    FreeList::Node instances_data;    
    size_t instances_count;
};

struct RendererContext
{
    DArray<DrawMesh> mesh_draws;
};