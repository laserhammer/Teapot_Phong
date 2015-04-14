#version 440

in vec3 position;
in vec3 normal;

uniform mat4 mpvMat;
uniform mat4 mpMat;
uniform vec4 color;
uniform vec4 camPos;

out vec4 Color;
out vec4 Normal;
out vec4 WorldPos;
out vec4 CamPos;

void main()
{
	Color = color;
	Normal = vec4(normal.xy, -normal.z, 0.0);
	WorldPos = mpvMat * vec4(position, 1.0);
	CamPos = vec4(camPos.xy, -camPos.z, camPos.w);
	gl_Position = WorldPos;
}