#version 460

layout(location=0) in vec2 vs_out_tex;
layout(location=0) out vec4 fs_out_col;
uniform sampler2D frameTex;
uniform sampler2D ssao;
uniform sampler2D diffuse;

void main()
{
    //SSAO BLUR
	vec2 texelSize = 1.0 / vec2(textureSize(ssao, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssao, vs_out_tex + offset).x;
        }
    }
    result = result * 0.0625; // result / 16

	fs_out_col = vec4(texture(frameTex, vs_out_tex).xyz + texture(diffuse, vs_out_tex).xyz * result * vec3(0.2),1);
    //fs_out_col = vec4(vec3(result),1);
}