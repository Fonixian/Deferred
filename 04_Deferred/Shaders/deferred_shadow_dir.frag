#version 460

//flat layout(location = 0) in vec3 color;
//flat layout(location = 1) in vec3 direction;
layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 fs_out_col;

layout(binding = 0) uniform sampler2D diffuseTexture;
layout(binding = 1) uniform sampler2D normalTexture;
layout(binding = 2) uniform sampler2D depthTexture;
layout(binding = 3) uniform sampler2DArray shadowTexture;

const float cascadePlaneDistances[5] = {1000.f / 50.0f, 1000.f / 25.0f, 1000.f / 10.0f, 1000.f / 2.0f, 1000.0f};
uniform mat4 lightSpaceMatrices[5];
uniform mat4 PI;
uniform mat4 VI;
uniform mat4 V;
uniform vec3 color;
uniform vec3 direction;

float calculate_shadow(){
	float d = texture(depthTexture, texCoord).x;
	vec4 fragPosView = PI * (vec4(vec3(texCoord, d) * 2. - 1., 1.));
	fragPosView /= fragPosView.w;

	float depthLenght = -fragPosView.z;

	int layer = -1;
	for(int i = 0; i < 5; ++i){
		if(depthLenght < cascadePlaneDistances[i]){
			layer = i;
			break;
		}
	}
	if (layer == -1) layer = 5;

	vec4 fragPosWorld = VI * fragPosView;

	vec4 fragPosLight = lightSpaceMatrices[layer] * fragPosWorld;
	fragPosLight /= fragPosLight.w;
	fragPosLight.xyz = fragPosLight.xyz * 0.5 + 0.5;

	//if(fragPosLight.x > 1 || fragPosLight.x < 0 || fragPosLight.y > 1 || fragPosLight.y < 0)
	//	return 1.0f;
	
	float bias = 0.001f;
	float shadow = 0.0f;
	vec2 texelSize = 1.0 / vec2(textureSize(shadowTexture, 0));
	for(int x = -1; x <= 1; ++x)
	{
	    for(int y = -1; y <= 1; ++y)
	    {
	        float pcfDepth = texture(
	                    shadowTexture,
	                    vec3(fragPosLight.xy + vec2(x, y) * texelSize,
	                    layer)
	                    ).r; 
	        shadow += fragPosLight.z - bias < pcfDepth ? 1.0 : 0.0;
	    }    
	}
	shadow /= 9.0;
	return shadow;
}

float calculate_shadow_linear(){
	float d = texture(depthTexture, texCoord).x;
	vec4 fragPosView = PI * (vec4(vec3(texCoord, d) * 2. - 1., 1.));
	fragPosView /= fragPosView.w;

	float depthLenght = -fragPosView.z;

	int layer = -1;
	for(int i = 0; i < 5; ++i){
		if(depthLenght < cascadePlaneDistances[i]){
			layer = i;
			break;
		}
	}
	if (layer == -1) layer = 5;

	vec4 fragPosWorld = VI * fragPosView;

	vec4 fragPosLight = lightSpaceMatrices[layer] * fragPosWorld;
	fragPosLight /= fragPosLight.w;
	fragPosLight.xyz = fragPosLight.xyz * 0.5 + 0.5;

	//if(fragPosLight.x > 1 || fragPosLight.x < 0 || fragPosLight.y > 1 || fragPosLight.y < 0)
	//	return 1.0f;
	
	float bias = 0.001f;
	bias *= layer;
	float shadow = 0.0f;
	vec2 texelSize = 1.0 / vec2(textureSize(shadowTexture, 0));
	for(int x = -1; x <= 1; ++x)
	{
	    for(int y = -1; y <= 1; ++y)
	    {
	        float pcfDepth = texture(
	                    shadowTexture,
	                    vec3(fragPosLight.xy + vec2(x, y) * texelSize,
	                    layer)
	                    ).r; 
	        shadow += fragPosLight.z - bias < pcfDepth ? 1.0 : 0.0;
	    }    
	}
	shadow /= 9.0;
	return shadow;
}

void main()
{
	vec3 lightDir = (V * vec4(direction,0)).xyz;

	vec4 Kd = texture( diffuseTexture, texCoord );
	vec3 n = normalize(texture( normalTexture, texCoord ).xyz);

	fs_out_col = calculate_shadow() * vec4(color,1)*(Kd*clamp(dot(n, lightDir), 0, 1));
}