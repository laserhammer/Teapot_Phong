#version 440

const int MAX_LIGHTS = 8;

struct Light
{
	vec4 position;
	vec4 color;
	vec4 ambient;
	float power;
};

in vec4 Color;
in vec4 Normal;
in vec4 WorldPos;
in vec4 CamPos;

uniform Light lights[MAX_LIGHTS];

out vec4 outColor;

void main()
{
	vec4 diffuse = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);
	for(int i = 0; i < MAX_LIGHTS; ++i)
	{
		vec4 lightDir = normalize(lights[i].position - WorldPos);
		float dis = length(lightDir);
		
		float NdotL = dot(-lightDir, Normal);
		float intensity = clamp(NdotL, 0.0, 1.0);
		diffuse += intensity * lights[i].color * lights[i].power / dis + lights[i].ambient;
		
		vec4 highlight = -reflect(lightDir, Normal);
		vec4 outVec = normalize(CamPos) - normalize(WorldPos);
		float specIntensity = pow(max(dot(highlight, outVec), 0.0), 0.15);
		specular += clamp(specIntensity * lights[i].color  * 0.25, 0.0, 1.0);
	}
	
	outColor =  specular + diffuse;
};