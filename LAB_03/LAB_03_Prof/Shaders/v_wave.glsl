// Vertex shader: Wave shading
// ================
#version 450 core

// Input vertex data, different for all executions of this shader.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

// Values that stay constant for the whole mesh.
uniform mat4 P;
uniform mat4 V;
uniform mat4 M; // position*rotation*scaling
uniform float T;

//uniform vec4 Color;
in vec4 Color;

void main()
{
    float a=0.1f;
    float w=0.0001f;
    vec3 v=aPos;
	v.y=a*sin(w*T+10*v.x)*sin(w*T+10*v.z);
    gl_Position = P * V * M * vec4(v, 1.0);
}