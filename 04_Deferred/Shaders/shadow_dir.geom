#version 460
    
layout(triangles, invocations = 5) in;
layout(triangle_strip, max_vertices = 3) out;
    
uniform mat4 lightSpaceMatrices[5];
uniform int update[5];

void main()
{
    if(update[gl_InvocationID] == 0) return;

    for (int i = 0; i < 3; ++i)
    {
        gl_Position = lightSpaceMatrices[gl_InvocationID] * gl_in[i].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}