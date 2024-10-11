#version 450
#extension GL_ARB_separate_shader_objects : enable

layout ( location = 0 ) out vec4 out_color;
layout ( location = 1 ) in struct dto
{
    vec2 texcoord;
    mat4 rect;
} in_dto;

layout ( set = 1 , binding = 0) uniform sampler2D diffuse_sampler;

void GetUVRange(float u , float uv_size , float tex_size , out vec2 uv_range, out vec2 tex_range)
{  
    float is_center_mul = float( u >= uv_size && u <= (1 - uv_size) );
    float is_right_mul = float(u < uv_size);
    float is_left_mul = float(u > (1 - uv_size));

    vec2 center_uv_range = vec2(uv_size , 1 - uv_size);
    vec2 center_tex_range = vec2(tex_size , 1 - tex_size);

    vec2 right_uv_range = vec2(0 , uv_size);
    vec2 right_tex_range = vec2(0 , tex_size);

    vec2 left_uv_range = vec2( 1 - uv_size , 1);
    vec2 left_tex_range = vec2( 1 - tex_size , 1);

    uv_range =  (is_center_mul * center_uv_range) + (is_right_mul * right_uv_range) + (is_left_mul * left_uv_range);  
    tex_range = (is_center_mul * center_tex_range) + (is_right_mul * right_tex_range) + (is_left_mul * left_tex_range);
}

float Remap(float val , vec2 old , vec2 new)
{
    float old_amp = old.y - old.x;
    float new_amp = new.y - new.x;
    float ratio = (val - old.x) / old_amp;
    return new.x + (ratio * new_amp);
}

void main ()
{    
    vec2 uv = in_dto.texcoord;
    uv.y = 1 - uv.y;

    // get corner size in UV space
    vec2 rect_size_px;
    rect_size_px.x = in_dto.rect[0][0];
    rect_size_px.y = in_dto.rect[1][1];

    vec2 rect_corner_in_pixels = vec2(32,32);
    vec2 rect_corner_in_uv;
    rect_corner_in_uv.x = rect_corner_in_pixels.x / rect_size_px.x;
    rect_corner_in_uv.y = rect_corner_in_pixels.y / rect_size_px.y;

    // define the corner size for the texture to sample
    vec2 tex_size_px = vec2(textureSize(diffuse_sampler,0));

    vec2 text_corner_in_px = vec2(50 , 50);
    vec2 tex_corner_in_uv;
    tex_corner_in_uv.x = text_corner_in_px.x / tex_size_px.x;  
    tex_corner_in_uv.y = text_corner_in_px.y / tex_size_px.y;

    // get the min-max range the UV should sample from in the texture
    vec2 uv_x_minmax , tex_x_minmax;
    vec2 uv_y_minmax , tex_y_minmax;
    GetUVRange(uv.x , rect_corner_in_uv.x , tex_corner_in_uv.x , uv_x_minmax , tex_x_minmax);
    GetUVRange(uv.y , rect_corner_in_uv.y , tex_corner_in_uv.y , uv_y_minmax , tex_y_minmax);

    vec2 uv_remapped;
    uv_remapped.x = Remap(uv.x , uv_x_minmax , tex_x_minmax);
    uv_remapped.y = Remap(uv.y , uv_y_minmax , tex_y_minmax);
    
    vec4 color = texture(diffuse_sampler , uv_remapped);
    
    out_color = color;
}