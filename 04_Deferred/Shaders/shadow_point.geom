#version 460

layout (triangles, invocations = 6) in;
layout (triangle_strip, max_vertices=3) out;

uniform mat4 shadowMatrices[6];
uniform int update[6];

layout(location = 0) out vec4 FragPos;

void main()
{
    if(update[gl_InvocationID] == 0) return;

    gl_Layer = gl_InvocationID;
    for(int i = 0; i < 3; ++i){
        FragPos = gl_in[i].gl_Position;
        gl_Position = shadowMatrices[gl_InvocationID] * gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}