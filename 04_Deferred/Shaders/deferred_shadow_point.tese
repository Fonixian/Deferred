#version 460

layout (quads, equal_spacing) in;

flat layout(location = 0) in vec3 color_in[];
flat layout(location = 1) in vec4 position_r_in[];

layout(location = 0) out vec3 color_out;
layout(location = 1) out vec4 position_r_out;
layout(location = 2) out vec2 texCoord;

uniform mat4 proj;

void main()
{
	vec2 uv = 3.1415f * gl_TessCoord.xy;
	float sv = sin(uv.y);
	//                      position in view                                      normal in view                           radius
	gl_Position = proj * ( (vec4(position_r_in[0].xyz,1) ) + vec4(vec3( cos(uv.x) * sv, cos(uv.y), -sin(uv.x)*sv ) * position_r_in[0].w,0) );

	texCoord = (gl_Position.xy / gl_Position.w ) * 0.5 + 0.5;
	color_out    = color_in[0];
	position_r_out = vec4(position_r_in[0].xyz, position_r_in[0].w * position_r_in[0].w );
}