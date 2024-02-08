#version 460

layout(vertices = 1) out;

flat layout(location = 0) in vec3 color_in[];
flat layout(location = 1) in vec4 position_r_in[];

layout(location = 0) out vec3 color_out[];
layout(location = 1) out vec4 position_r_out[];

void main()
{
	gl_TessLevelInner[0] = 3;
	gl_TessLevelInner[1] = 3;

	gl_TessLevelOuter[0] = 3;
	gl_TessLevelOuter[1] = 1;
	gl_TessLevelOuter[2] = 3;
	gl_TessLevelOuter[3] = 1;

	color_out[gl_InvocationID]      = color_in[gl_InvocationID];
	position_r_out[gl_InvocationID] = position_r_in[gl_InvocationID];
}