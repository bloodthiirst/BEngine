#version 450
#extension GL_ARB_separate_shader_objects : enable

layout ( location = 0 ) out vec4 out_color;
layout ( location = 1 ) in struct dto
{
    vec4 out_font_uv;
    vec2 texcoord;
} in_dto;

layout ( set = 1 , binding = 0) uniform sampler2D diffuse_sampler;

void main ()
{
    vec2 atlast_uv = vec2(0,0);
    atlast_uv.x = in_dto.out_font_uv.x + (in_dto.texcoord.x * in_dto.out_font_uv.z);
    atlast_uv.y = in_dto.out_font_uv.y + ( ( 1 - in_dto.texcoord.y) * in_dto.out_font_uv.w);
    
    vec4 col = texture(diffuse_sampler , atlast_uv);
    out_color = col;
}