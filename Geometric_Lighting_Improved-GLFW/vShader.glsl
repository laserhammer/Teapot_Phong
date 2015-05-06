#version 440

in vec3 position;
in vec3 normal;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 projMat;
uniform vec4 color;
uniform vec4 camPos;

out vec4 Color;
out vec4 Normal;
out vec4 WorldPos;
out vec4 CamPos;

void main()
{
	Color = color;
	Normal =  transpose(inverse(modelMat)) * vec4(normal.xyz, 0.0);
	WorldPos = modelMat * vec4(position.xyz, 1.0);
	CamPos = camPos;
	gl_Position = projMat * viewMat * modelMat * vec4(position.xyz, 1.0);
}