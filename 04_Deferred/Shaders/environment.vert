#version 460

// incoming vertex attributes from the VBO via the VAO
// now with explicit location!
layout(location=0) in vec3 vs_in_pos;
layout(location=1) in vec3 vs_in_normal;
layout(location=2) in vec2 vs_in_tex0;

layout(location=0) out vec2 vs_out_tex0;

// transformation this shader need to perform
uniform mat4 world;
//uniform mat4 worldIT;
//uniform mat4 MVP;
//uniform mat4 MVIT;

void main()
{
	gl_Position   = world * vec4( vs_in_pos, 1 );
	vs_out_tex0   = vs_in_tex0;
}