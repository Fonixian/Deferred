#version 460

struct Light{
	vec4 color;
	vec4 direction;
};

const vec4 positions[4] = vec4[4](
	vec4(-1,-1, 0, 1),
	vec4( 1,-1, 0, 1),
	vec4(-1, 1, 0, 1),
	vec4( 1, 1, 0, 1)
);

const vec2 texCoords[4] = vec2[4](
	vec2(0,0),
	vec2(1,0),
	vec2(0,1),
	vec2(1,1)
);

layout(location = 0) out vec3 color_out;
layout(location = 1) out vec3 direction_out;
layout(location = 2) out vec2 vs_out_tex;



restrict layout(std430, binding = 0) buffer positionBuffer
{
	Light lights[];
};

uniform mat4 VIT;

void main()
{
	color_out = lights[gl_InstanceID].color.xyz;
	direction_out = normalize((VIT * lights[gl_InstanceID].direction).xyz);

	gl_Position = positions[gl_VertexID];
	vs_out_tex	= texCoords[gl_VertexID];
}