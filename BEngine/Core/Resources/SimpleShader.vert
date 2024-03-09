#version 450
#extension GL_ARB_separate_shader_objects : enable

layout ( location = 0) in vec3 in_position;
layout ( location = 1) in vec2 in_texcoord;

layout ( location = 0 ) out vec3 out_position;
layout ( location = 1 ) out struct dto
{
    vec2 out_texcoord;
} out_dto;

layout (set = 0, binding = 0) uniform global_uniform_object {
    mat4 projection;
    mat4 view;
} global_ubo;

void main ()
{
    vec4 pos =  vec4(in_position, 1.0) * global_ubo.view * global_ubo.projection;
	
    gl_Position = pos;

    out_position = pos.xyz;
    out_dto.out_texcoord = in_texcoord;
}