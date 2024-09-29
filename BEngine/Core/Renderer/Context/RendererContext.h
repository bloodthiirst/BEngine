#pragma once
#include <Containers/DArray.h>
#include "../Shader/Shader.h"
#include "../Mesh/Mesh3D.h"


struct DrawMesh
{
    Mesh3D* mesh;
    ShaderBuilder* shader_builder;
    Texture* texture;
    Buffer instances_data;    
    size_t instances_count;
};

struct RendererContext
{
    DArray<DrawMesh> mesh_draws;
};