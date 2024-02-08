#version 460

layout(location=0) in vec2 vs_out_tex;
layout(location=0) out float fs_out_col;

layout(binding = 0) uniform sampler2D normalTexture;
layout(binding = 1) uniform sampler2D depthTexture;
layout(binding = 2) uniform sampler2D noiseTexture;

const float radius = 0.5;
const int kernelSize = 64;
const float bias = 0.025;

uniform mat4 PI;
uniform mat4 P;
uniform vec2 noiseScale = vec2(1080.0/4.0, 760.0/4.0);
uniform vec3 samples[kernelSize];

vec4 getPosView(vec2 coord){
    float depth = texture(depthTexture, coord).x;
    vec4 fragPos = PI * vec4(vec3(coord, depth) * 2. - 1., 1.);
    return fragPos /= fragPos.w;
}

void main(){
	float depth = texture(depthTexture, vs_out_tex).x;
	///
    if(depth == 1.0){
        fs_out_col = 0.;
        return;
    }
    ///
    vec4 fragPos = PI * vec4(vec3(vs_out_tex, depth) * 2. - 1., 1.);
    fragPos /= fragPos.w;

    vec3 normal = texture(normalTexture, vs_out_tex).xyz;

    vec3 randomVec = texture(noiseTexture, vs_out_tex * noiseScale).xyz;

    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i){
        vec3 samplePos = TBN * samples[i];
        samplePos = fragPos.xyz + samplePos * radius; 

        vec4 offset = vec4(samplePos, 1.0);
        offset      = P * offset;
        offset.xyz /= offset.w;
        offset.xyz  = offset.xyz * 0.5 + 0.5;

        float depth = texture(depthTexture, offset.xy).x;
        vec4 offsetPos = PI * vec4(vec3(offset.xy, depth) * 2. - 1., 1.);
        offsetPos /= offsetPos.w;

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - offsetPos.z));
        occlusion += (offsetPos.z >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck; 
    }

    fs_out_col = 1.0 - (occlusion / kernelSize);
}