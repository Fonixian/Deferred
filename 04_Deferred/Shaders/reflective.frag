#version 460

//out vec3 vs_out_pos;
layout(location=0) in vec3 in_normal;
layout(location=1) in vec2 in_tex0;
layout(location=2) in vec3 pos_view;

// multiple outputs are directed into different color textures by the FBO
layout(location=0) out vec4 out_diffuse;
layout(location=1) out vec3 out_normal;

// Different geometries may be drawn with different textures attached
layout(binding = 0) uniform sampler2D texImage;
layout(binding = 1) uniform samplerCube environmentMap;

uniform mat4 VI;


vec3 blend_screen(vec3 a, vec3 b) {
	return a + b - a * b;
}

void main(void) {
	out_normal = normalize(in_normal);
	vec3 sampleDir = (VI * vec4(reflect(pos_view,out_normal),0)).xyz;

	out_diffuse = vec4(blend_screen( texture(texImage, in_tex0).xyz, texture(environmentMap, sampleDir).xyz ),1);
}