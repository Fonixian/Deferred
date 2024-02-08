#version 420

layout(vertices = 16) out;

in block
{
	vec3	pos;
} In[];

out block
{
	vec3	pos;
} Out[];

uniform float tess_level = 32;

void main()
{
	Out[ gl_InvocationID ].pos = In[ gl_InvocationID ].pos;

	if ( 0 == gl_InvocationID )
	{
		gl_TessLevelInner[0] = tess_level;
		gl_TessLevelInner[1] = tess_level;

		gl_TessLevelOuter[0] = tess_level;
		gl_TessLevelOuter[1] = tess_level;
		gl_TessLevelOuter[2] = tess_level;
		gl_TessLevelOuter[3] = tess_level;
	}
}