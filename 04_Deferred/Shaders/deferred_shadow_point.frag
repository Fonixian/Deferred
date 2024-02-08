#version 460

flat layout(location = 0) in vec3 color;
flat layout(location = 1) in vec4 position_r;
noperspective layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec4 fs_out_col;

layout(binding = 1) uniform sampler2D diffuseTexture;
layout(binding = 2) uniform sampler2D normalTexture;
layout(binding = 3) uniform sampler2D depthTexture;

uniform vec3 position_in;
uniform mat4 PI;
uniform mat4 VI;
uniform float radius;
layout(binding = 0) uniform samplerCube depthMap;

const float bias    = 0.05; 
const float samples = 4.0;
const float offset  = 0.08;

float shadowCalcutaion(vec4 fragPosView,float currentDepth){
	vec3 fragToLight = (VI * fragPosView).xyz - position_in.xyz;
	float shadow  = 0.0;

	for(float x = -offset; x < offset; x += offset / (samples * 0.5))
		for(float y = -offset; y < offset; y += offset / (samples * 0.5))
			for(float z = -offset; z < offset; z += offset / (samples * 0.5)){
				float closestDepth = texture(depthMap, fragToLight + vec3(x, y, z)).r;
				closestDepth *= radius;
				if(currentDepth - bias < closestDepth) shadow += 1.0;
			}
	shadow /= (samples * samples * samples);
	return shadow;
}

vec4 getInView(in float depth){
	vec4 position_view = PI * (vec4(vec3(texCoord, depth) * 2. - 1., 1.));
	return position_view / position_view.w;
}

void main()
{
	float d = texture(depthTexture, texCoord).x;
	//vec4 pos = PI * (vec4(vec3(texCoord, d) * 2. - 1., 1.));
	//pos /= pos.w;

	vec4 pos = getInView(d);

	////////////////////////////////////
	//vec3 diff = pos.xyz - position_r.xyz;
	//if(dot(diff,diff) > position_r.w)discard;
	////////////////////////////////////

	vec3 light = position_r.xyz - pos.xyz;
	vec3 lightDir = normalize(light);

	vec4 Kd = texture( diffuseTexture, texCoord );
	vec3 n = texture( normalTexture, texCoord ).rgb;

	fs_out_col = shadowCalcutaion(pos,length(light)) * vec4(color,1) * (Kd*clamp(dot(n, lightDir), 0, 1)) / dot(light,light);
}