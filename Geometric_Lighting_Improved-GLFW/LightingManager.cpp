#include "LightingManager.h"
#include "RenderManager.h"
#include <string>

Light LightingManager::_lights[MAX_LIGHTS];
RenderShape* LightingManager::_lightShapes[MAX_LIGHTS];

GLint LightingManager::_shader;
GLuint LightingManager::_uLights;

void LightingManager::Init(Shader lightingShader)
{
	_shader = lightingShader.shaderPointer;
	glUseProgram(_shader);
	for (int i = 0; i < MAX_LIGHTS; ++i)
	{
		_lights[i] = Light();
		_lights[i].position = glm::vec3();
		_lights[i].linearVelocity = glm::vec3();
		_lights[i].rotation = glm::quat();
		_lights[i].rotationOrigin = glm::vec3();
		_lights[i].angularVelocity = glm::quat();
		_lights[i].transformPos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		_lights[i].color = glm::vec4();
		_lights[i].power = 0.0f;
		_lights[i].active = false;

		std::string currentLight = "lights[";
		char numStringBuffer[2];
		itoa(i, numStringBuffer, 10);
		currentLight += numStringBuffer;
		currentLight += "].";
		std::string pos = "position";
		std::string col = "color";
		std::string pow = "power";
		std::string amb = "ambient";
		_lights[i].uPosition = glGetUniformLocation(_shader, (currentLight + pos).c_str());
		_lights[i].uColor = glGetUniformLocation(_shader, (currentLight + col).c_str());
		_lights[i].uPower = glGetUniformLocation(_shader, (currentLight + pow).c_str());
		_lights[i].uAmbient = glGetUniformLocation(_shader, (currentLight + amb).c_str());
	}	
}
void LightingManager::Update(float dt)
{
	glUseProgram(_shader);
	for (int i = 0; i < MAX_LIGHTS; ++i)
	{
		if (_lights[i].active)
		{
			_lights[i].position += _lights[i].linearVelocity * dt;
			_lights[i].rotation = glm::slerp(_lights[i].rotation, _lights[i].rotation * _lights[i].angularVelocity, dt);

			glm::mat4 translateMat = glm::translate(glm::mat4(), _lights[i].position);

			glm::mat4 rotateOriginMat = glm::translate(glm::mat4(), _lights[i].rotationOrigin);
			glm::mat4 rotateMat = rotateOriginMat * glm::mat4_cast(_lights[i].rotation) * glm::inverse(rotateOriginMat);

			_lights[i].transformPos = translateMat * rotateMat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

			glUniform4fv(_lights[i].uPosition, 1, glm::value_ptr(_lights[i].transformPos));
			glUniform4fv(_lights[i].uColor, 1, glm::value_ptr(_lights[i].color));
			glUniform1f(_lights[i].uPower, _lights[i].power);
			glUniform4fv(_lights[i].uAmbient, 1, glm::value_ptr(_lights[i].ambient));
		}

		if (_lightShapes[i])
		{
			_lightShapes[i]->active() = _lights[i].active;
			_lightShapes[i]->transform().position = glm::vec3(_lights[i].transformPos);
			_lightShapes[i]->currentColor() = _lights[i].color;
		}
	}
}
Light& LightingManager::GetLight(int index)
{
	return _lights[index];
}

void LightingManager::SetLightShape(RenderShape* shape, int index)
{
	_lightShapes[index] = shape;
	RenderManager::AddShape(shape);
}

	