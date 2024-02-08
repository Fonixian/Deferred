#version 460

struct Light{
	vec4 color;
	vec4 position;
};

layout(location = 0) out vec3 color_out;
layout(location = 1) out vec4 position_r_out;

restrict layout(std430, binding = 0) buffer positionBuffer
{
	Light lights[];
};

uniform mat4 view;

void main()
{
	//float radius = lights[gl_VertexID].color.r + lights[gl_VertexID].color.g + lights[gl_VertexID].color.b;
	//radius = sqrt(radius / 0.05);


	color_out = lights[gl_VertexID].color.rgb;
	vec4 pView = view * lights[gl_VertexID].position;
	position_r_out = vec4(pView.xyz, lights[gl_VertexID].color.w);
}