#version 460

flat layout(location = 0) in vec3 color;
flat layout(location = 1) in vec4 position_r;
noperspective layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec4 fs_out_col;

layout(binding = 1) uniform sampler2D diffuseTexture;
layout(binding = 2) uniform sampler2D normalTexture;
layout(binding = 3) uniform sampler2D depthTexture;

uniform mat4 PI;

void main()
{
	float d = texture(depthTexture, texCoord).x;
	vec4 pos = PI * (vec4(vec3(texCoord, d) * 2. - 1., 1.));
	pos.xyz /= pos.w;

	////////////////////////////////////
	//vec3 diff = pos.xyz - position_r.xyz;
	//if(dot(diff,diff) > position_r.w)discard;
	////////////////////////////////////

	vec3 light = position_r.xyz - pos.xyz;
	vec3 lightDir = normalize(light);

	vec4 Kd = texture( diffuseTexture, texCoord );
	vec3 n = texture( normalTexture, texCoord ).rgb;

	fs_out_col = vec4(color,1)*(Kd*clamp(dot(n, lightDir), 0, 1)) / dot(light,light);
}