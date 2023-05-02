#version 450

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

layout (push_constant) uniform Constants
{
	mat4 model;
}u_ObjectData;

layout(location = 0) out vec3 fragColor;

void main()
{
	gl_Position = u_ObjectData.model * vec4(aPos, 0.0, 1.0);
	fragColor = aColor;
}