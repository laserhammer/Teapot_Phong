#version 440

in vec3 position;

uniform mat4 mpvMat;
uniform vec4 color;

out vec4 Color;

void main()
{
	Color = color;
	gl_Position = mpvMat * vec4(position, 1.0);
}