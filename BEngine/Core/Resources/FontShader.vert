#version 450
#extension GL_ARB_separate_shader_objects : enable

layout ( location = 0) in vec3 in_position;
layout ( location = 1) in vec2 in_texcoord;

layout ( location = 0 ) out vec3 out_position;
layout ( location = 1 ) out struct dto
{
    vec4 out_font_uv;
    vec2 out_texcoord;
} out_dto;

layout (set = 0, binding = 0) uniform global_uniform_object {
    mat4 projection;
    mat4 view;
    float time;
} global_ubo;

struct FontData
{
    mat4 mat;
    vec4 uv_data;
};

//all object matrices
layout(set = 2, binding = 0) uniform InstanceBuffer {
	FontData font_data[100];
} instances_buffer;

void main ()
{
    FontData data = instances_buffer.font_data[gl_InstanceIndex];
    vec4 pos =  vec4(in_position , 1.0) * data.mat * global_ubo.view * global_ubo.projection;

    out_position = pos.xyz;
    out_dto.out_texcoord = in_texcoord;
    out_dto.out_font_uv = data.uv_data;
    gl_Position = pos;
}