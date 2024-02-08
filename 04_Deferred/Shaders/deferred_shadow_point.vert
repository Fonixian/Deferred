#version 460

uniform vec3 color_in;
uniform vec3 position_in;

layout(location = 0) out vec3 color_out;
layout(location = 1) out vec4 position_r_out;

uniform mat4 view;
uniform float radius;

void main()
{
	color_out = color_in;
	vec4 pView = view * vec4(position_in,1);
	position_r_out = vec4(pView.xyz, radius);
}