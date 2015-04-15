#pragma once
#include <GLEW\GL\glew.h>
#include <GLM\gtc\matrix_transform.hpp>
#include <GLM\gtc\quaternion.hpp>
#include "RenderShape.h"
#include "InteractiveShape.h"
struct Light
{
	glm::vec3 position;
	glm::vec3 linearVelocity;
	glm::quat rotation;
	glm::vec3 rotationOrigin;
	glm::quat angularVelocity;
	glm::vec4 ambient;
	GLint uAmbient;
	glm::vec4 color;
	GLint uColor;
	glm::vec4 transformPos;
	GLint uPosition;
	float power;
	GLint uPower;
	bool active;
};

class LightingManager
{
public:
	static void Init(Shader lightingShader);
	static void Update(float dt);
	static Light& GetLight(int index);
	static void SetLightShape(RenderShape* shape, int index);
private:
	static const int MAX_LIGHTS = 8;

	static Light _lights[MAX_LIGHTS];
	static GLfloat _lightBufferData[MAX_LIGHTS * 8];
	static RenderShape* _lightShapes[MAX_LIGHTS];

	static GLint _shader;
	static GLuint _uLights;
};