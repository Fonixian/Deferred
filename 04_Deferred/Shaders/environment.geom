#version 460

layout (triangles, invocations = 6) in;
layout (triangle_strip, max_vertices=3) out;

layout(location=0) in vec2 in_tex0[];

layout(location=0) out vec2 out_tex0;

uniform mat4 transforms[6];

//layout (std140, binding = 0) uniform test{
//    mat4 transforms[6];
//};

uniform int update[6];

void main()
{
    if(update[gl_InvocationID] == 0) return;

    gl_Layer = gl_InvocationID;
    for(int i = 0; i < 3; ++i){
        out_tex0 = in_tex0[i];

        gl_Position = transforms[gl_InvocationID] * gl_in[i].gl_Position;

        EmitVertex();
    }
    EndPrimitive();
}