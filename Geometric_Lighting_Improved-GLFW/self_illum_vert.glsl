#version 440

in vec3 position;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;
uniform vec4 color;

out vec4 Color;

void main()
{
	Color = color;
	gl_Position = projMat * viewMat * modelMat * vec4(position.xyz, 1.0);
}