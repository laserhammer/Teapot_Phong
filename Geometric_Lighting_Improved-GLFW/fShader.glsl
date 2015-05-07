#version 440

const int MAX_LIGHTS = 8;

struct Light
{
	vec4 position;
	vec4 color_difPower;
};

in vec4 Color;
in vec4 Normal;
in vec4 WorldPos;
in vec4 CamPos;

uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;

out vec4 outColor;

void main()
{
	vec4 diffuse = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 lightColor, lightVec, lightDir, highlight, outVec, specInt;
	float dis, NdotL, lightPower;
	for(int i = 0; i < MAX_LIGHTS; ++i)
	{
		lightColor = vec4(lights[i].color_difPower.xyz, 1.0);
		lightPower = lights[i].color_difPower.w;
		lightVec = vec4(lights[i].position.xyz - WorldPos.xyz, 0.0);
		lightDir = normalize(lightVec);
		
		dis = length(lightVec);
		NdotL = dot(Normal, lightDir);
		diffuse += clamp(NdotL, 0.0, 1.0) * lightColor * lightPower / (dis * dis);
		
		highlight = reflect(-lightDir, Normal);
		outVec = normalize(CamPos - WorldPos);
		specular += lightColor * pow(max(dot(highlight, outVec), 0.0), 3.0);
	}
	
	outColor = specular + (diffuse + ambient) * Color;
};