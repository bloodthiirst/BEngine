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

    // NOTE : we do this since vulkan use top-left corner as (0,0) 
    // which means that U goes to the right and V goes down , so we do a remapping to use the bottom-left as (0,0)
    atlast_uv.y = (1 - in_dto.out_font_uv.y) + ( ( 1 - in_dto.texcoord.y) * in_dto.out_font_uv.w);
    
    vec4 col = texture(diffuse_sampler , atlast_uv);
    col.a = col.r;
    out_color = col;
}