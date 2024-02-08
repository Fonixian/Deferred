#version 460

//out vec3 vs_out_pos;
layout(location=0) in vec3 vs_out_normal;
layout(location=1) in vec2 vs_out_tex0;

// multiple outputs are directed into different color textures by the FBO
layout(location=0) out vec4 fs_out_diffuse;
layout(location=1) out vec3 fs_out_normal;

// Different geometries may be drawn with different textures attached
uniform sampler2D texImage;

void main(void) {
	fs_out_diffuse = vec4(texture(texImage, vs_out_tex0).xyz, 1);
	fs_out_normal = normalize(vs_out_normal);
}