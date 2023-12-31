#version 450
#extension GL_ARB_separate_shader_objects : enable

layout ( location = 0 ) in vec3 in_position;

layout ( location = 0 ) out vec3 out_position;

layout (set = 0, binding = 0) uniform global_uniform_object {
    mat4 projection;
    mat4 view;
} global_ubo;

void main ()
{
    vec4 pos = vec4(in_position, 1.0) * global_ubo.view * global_ubo.projection;
    //vec3 pos = in_position;
    out_position = pos.xyz;
	gl_Position = pos;
}