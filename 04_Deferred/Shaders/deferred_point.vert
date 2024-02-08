#version 460

layout(location = 0) in vec3 color_in;
layout(location = 1) in vec3 position_in;

layout(location = 0) out vec3 color_out;
layout(location = 1) out vec4 position_r_out;

uniform mat4 view;

void main()
{
	float radius = color_in.r + color_in.g + color_in.b;
	radius = sqrt(radius / 0.05);


	color_out = color_in;
	vec4 pView = view * vec4(position_in.xyz,1);
	position_r_out = vec4(pView.xyz, radius);
}