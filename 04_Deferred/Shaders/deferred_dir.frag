#version 460

flat layout(location = 0) in vec3 color;
flat layout(location = 1) in vec3 direction;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec4 fs_out_col;

layout(binding = 1) uniform sampler2D diffuseTexture;
layout(binding = 2) uniform sampler2D normalTexture;

void main()
{
	vec3 lightDir = direction;

	vec4 Kd = vec4(texture( diffuseTexture, texCoord ).xyz,1);
	vec3 n = normalize(texture( normalTexture, texCoord ).xyz);
	
	fs_out_col = vec4(color,1)*(Kd*clamp(dot(n, lightDir), 0, 1));
}