#version 450

layout (quads, fractional_even_spacing) in;

in block
{
	vec3	pos;
} In[];

out block
{
	vec3	position;
	vec3	normal;
	vec2	texcoord;
} Out;

uniform mat4 VP;

// P(u,v) --> R^3
//P' : R^3x2

void main()
{
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;

	float Bu[4] = {(1-u)*(1-u)*(1-u),3*u*(1-u)*(1-u),3*u*u*(1-u),u*u*u};
	float Bv[4] = {(1-v)*(1-v)*(1-v),3*v*(1-v)*(1-v),3*v*v*(1-v),v*v*v};

	float dBu[3] = {(1-u)*(1-u), 2*u*(1-u),u*u};
	float dBv[3] = {(1-v)*(1-v), 2*v*(1-v),v*v};

	vec3 dub[3*4], dvb[4*3];

	for (int i=0; i<3; ++i)
		for (int j=0; j<4; ++j)
			dub[i + j*3] = 3*(In[i+1 + j*4].pos - In[i + j*4].pos);

	for (int i=0; i<4; ++i)
		for (int j=0; j<3; ++j)
			dvb[i + j*4] = 3*(In[i + (j+1)*4].pos - In[i + j*4].pos);
	// evaluation of b(u,v)
	Out.position = vec3( 0, 0, 0 );

	for (int i=0; i<4; ++i)
	{
		for (int j=0; j<4; ++j)
		{
			Out.position += In[i + j*4].pos*Bu[i]*Bv[j];
		}
	}
	// evaluation of partial derivatives u, v
	vec3 du = vec3(0), dv = vec3(0);

	for (int i=0; i<3; ++i)
		for (int j=0; j<4; ++j)
			du += dub[i + j*3]*dBu[i]*Bv[j];

	for (int i=0; i<4; ++i)
		for (int j=0; j<3; ++j)
			dv += dvb[i + j*4]*Bu[i]*dBv[j];
	vec3 n = normalize( cross(du, dv) );

	gl_Position = VP * vec4(Out.position, 1);
	Out.normal		= n;
	Out.texcoord		= gl_TessCoord.xy;
}