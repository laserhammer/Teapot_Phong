#include <GLEW\GL\glew.h>
#include <GLFW\glfw3.h>
#include <GLM\gtc\type_ptr.hpp>
#include <GLM\gtc\matrix_transform.hpp>
#include <GLM\gtc\quaternion.hpp>
#include <GLM\gtc\random.hpp>
#include <iostream>
#include <ctime>

#include "RenderShape.h"
#include "InteractiveShape.h"
#include "Init_Shader.h"
#include "RenderManager.h"
#include "InputManager.h"
#include "B-Spline.h"
#include "Patch.h"
#include "CameraManager.h"
#include "LightingManager.h"

GLFWwindow* window;

GLuint vertexShader;
GLuint fragmentShader;
GLuint shaderProgram;

GLuint selfIllumVertShader;
GLuint selfIllumFragShader;
GLuint selfIllumProgram;

GLint posAttrib;
GLint uMPMat;
GLint uMPVMat;
GLint uColor;
GLint uCamPos;

GLint posAttrib_SI;
GLint uMPVMat_SI;
GLint uColor_SI;

GLuint cubeVAO;
GLuint cubeVBO;
GLuint cubeEBO;

GLfloat vertices[] = {
	-1.0f, +1.0f, -1.0f,
	+1.0f, +1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	+1.0f, -1.0f, -1.0f,
	-1.0f, +1.0f, +1.0f,
	+1.0f, +1.0f, +1.0f,
	-1.0f, -1.0f, +1.0f,
	+1.0f, -1.0f, +1.0f
};

GLint elements[] = {
	// Front
	0, 1, 2,
	1, 3, 2,

	// Back
	6, 5, 4,
	6, 7, 5,

	// Top
	4, 5, 0,
	1, 0, 5,

	// Bottom
	7, 2, 3,
	7, 6, 2,

	// Right
	1, 5, 7,
	7, 3, 1,

	// Left
	6, 4, 0,
	0, 2, 6,
};


// Source http://www.holmes3d.net/graphics/teapot/teapotCGA.bpt
GLfloat teapotControlPoints[] = {
	1.4, 2.25, 0,		1.3375,	2.38125, 0,			1.4375,	2.38125, 0,			1.5, 2.25, 0,
	1.4, 2.25, .784,	1.3375,	2.38125, .749,		1.4375, 2.38125, .805,		1.5, 2.25, .84,
	.784, 2.25, 1.4,	.749, 2.38125, 1.3375,		.805, 2.38125, 1.4375,		.84, 2.25, 1.5,
	0, 2.25, 1.4,		0, 2.38125, 1.3375,			0, 2.38125, 1.4375,			0, 2.25, 1.5,

	0, 2.25, 1.4,		0, 2.38125, 1.3375,			0, 2.38125, 1.4375,			0, 2.25, 1.5,
	-.784, 2.25, 1.4,	-.749, 2.38125, 1.3375,		-.805, 2.38125,	1.4375,		-.84, 2.25, 1.5,
	-1.4, 2.25, .784,	-1.3375, 2.38125, .749,		-1.4375, 2.38125, .805,		-1.5, 2.25, .84,
	-1.4, 2.25, 0,		-1.3375, 2.38125, 0,		-1.4375, 2.38125, 0,		-1.5, 2.25, 0,

	-1.4, 2.25, 0,		-1.3375, 2.38125, 0,		-1.4375, 2.38125, 0,		-1.5, 2.25, 0,
	-1.4, 2.25, -.784,	-1.3375, 2.38125, -.749,	-1.4375, 2.38125, -.805,	-1.5, 2.25, -.84,
	-.784, 2.25, -1.4,	-.749, 2.38125, -1.3375,	-.805, 2.38125,	-1.4375,	-.84, 2.25, -1.5,
	0, 2.25, -1.4,		0, 2.38125, -1.3375,		0,	2.38125, -1.4375,		0, 2.25, -1.5,

	0, 2.25, -1.4,		0, 2.38125, -1.3375,		0, 2.38125, -1.4375,		0, 2.25, -1.5,
	.784, 2.25, -1.4,	.749, 2.38125, -1.3375,		.805, 2.38125, -1.4375,		.84, 2.25, -1.5,
	1.4, 2.25, -.784,	1.3375,	2.38125, -.749,		1.4375, 2.38125, -.805,		1.5, 2.25, -.84,
	1.4, 2.25, 0,		1.3375,	2.38125, 0,			1.4375, 2.38125, 0,			1.5, 2.25, 0,

	1.5, 2.25, 0,		1.75, 1.725, 0,				2, 1.2,	0,					2, .75, 0,
	1.5, 2.25, .84,		1.75, 1.725, .98,			2, 1.2,	1.12,				2, .75, 1.12,
	.84, 2.25, 1.5,		.98, 1.725, 1.75,			1.12, 1.2, 2,				1.12, .75, 2,
	0, 2.25, 1.5,		0, 1.725, 1.75,				0, 1.2,	2,					0, .75, 2,

	0, 2.25, 1.5,		0, 1.725, 1.75,				0, 1.2,	2,					0, .75, 2,
	-.84, 2.25, 1.5,	-.98, 1.725, 1.75,			-1.12, 1.2, 2,				-1.12, .75, 2,
	-1.5, 2.25, .84,	-1.75, 1.725, .98,			-2, 1.2, 1.12,				-2, .75, 1.12,
	-1.5, 2.25, 0,		-1.75, 1.725, 0,			-2, 1.2, 0,					-2, .75, 0,

	-1.5, 2.25, 0,		-1.75, 1.725, 0,			-2, 1.2, 0,					-2, .75, 0,
	-1.5, 2.25, -.84,	-1.75, 1.725, -.98,			-2, 1.2, -1.12,				-2, .75, -1.12,
	-.84, 2.25, -1.5,	-.98, 1.725, -1.75,			-1.12, 1.2, -2,				-1.12, .75, -2,
	0, 2.25, -1.5,		0, 1.725,-1.75,				0, 1.2, -2,					0, .75, -2,

	0, 2.25, -1.5,		0, 1.725, -1.75,			0, 1.2,	-2,					0, .75, -2,
	.84, 2.25, -1.5,	.98, 1.725, -1.75,			1.12, 1.2, -2,				1.12, .75, -2,
	1.5, 2.25, -.84,	1.75, 1.725, -.98,			2, 1.2, -1.12,				2, .75, -1.12,
	1.5, 2.25, 0,		1.75, 1.725, 0,				2, 1.2, 0,					2, .75, 0,

	2, .75, 0,			2, .3, 0,					1.5, .075, 0,				1.5, 0, 0,
	2, .75, 1.12,		2, 	.3, 1.12,				1.5, .075, .84,				1.5, 0, .84,
	1.12, .75, 2,		1.12, .3, 2,				.84, .075, 1.5,				.84, 0, 1.5,
	0, .75, 2,			0, .3, 2,					0, .075, 1.5,				0, 0, 1.5,

	0, .75, 2,			0, .3, 2,					0, .075, 1.5,				0, 0, 1.5,
	-1.12, .75, 2,		-1.12, .3, 2,				-.84, .075, 1.5,			-.84, 0, 1.5,
	-2, .75, 1.12,		-2, .3, 1.12,				-1.5, .075,	.84,			-1.5, 0, .84,
	-2, .75, 0,			-2, .3, 0,					-1.5, .075,	0,				-1.5, 0, 0,

	-2, .75, 0,			-2,	.3, 0,					-1.5, .075,	0,				-1.5, 0, 0,
	-2, .75, -1.12,		-2,	.3, -1.12,				-1.5, .075,	-.84,			-1.5, 0, -.84,
	-1.12, .75, -2,		-1.12, .3, -2,				-.84, .075,	-1.5,			-.84, 0, -1.5,
	0, .75, -2,			0, .3, -2,					0, .075, -1.5,				0, 0, -1.5,

	0, .75, -2,			0, .3, -2,					0, .075, -1.5,				0, 0, -1.5,
	1.12, .75, -2,		1.12, .3, -2,				.84, .075, -1.5,			.84, 0, -1.5,
	2, .75, -1.12,		2, .3, -1.12,				1.5, .075,	-.84,			1.5, 0, -.84,
	2, .75, 0,			2, .3, 0,					1.5, .075,	0,				1.5, 0, 0,

	-1.6, 1.875, 0,		-2.3, 1.875, 0,				-2.7, 1.875, 0,				-2.7, 1.65, 0,
	-1.6, 1.875, .3,	-2.3, 1.875, .3,			-2.7, 1.875, .3,			-2.7, 1.65, .3,
	-1.5, 2.1, .3,		-2.5, 2.1, .3,				-3, 2.1, .3,				-3, 1.65, .3,
	-1.5, 2.1, 0,		-2.5, 2.1, 0,				-3, 2.1, 0,					-3, 1.65, 0,

	-1.5, 2.1, 0,		-2.5, 2.1, 0,				-3, 2.1, 0,					-3, 1.65, 0,
	-1.5, 2.1, -.3,		-2.5, 2.1, -.3,				-3, 2.1, -.3,				-3, 1.65, -.3,
	-1.6, 1.875, -.3,	-2.3, 1.875, -.3,			-2.7, 1.875, -.3,			-2.7, 1.65, -.3,
	-1.6, 1.875, 0,		-2.3, 1.875, 0,				-2.7, 1.875, 0,				-2.7, 1.65, 0,

	-2.7, 1.65, 0,		-2.7, 1.425, 0,				-2.5, .975, 0,				-2, .75, 0,
	-2.7, 1.65, .3,		-2.7, 1.425, .3,			-2.5, .975,	.3,				-2, .75, .3,
	-3, 1.65, .3,		-3, 1.2, .3,				-2.65, .7875, .3,			-1.9, .45, .3,
	-3, 1.65, 0,		-3,	1.2, 0,					-2.65, .7875, 0,			-1.9, .45, 0,

	-3, 1.65, 0,		-3,	1.2, 0,					-2.65, .7875, 0,			-1.9, .45, 0,
	-3, 1.65, -.3,		-3,	1.2, -.3,				-2.65, .7875, -.3,			-1.9, .45, -.3,
	-2.7, 1.65, -.3,	-2.7, 1.425, -.3,			-2.5, .975, -.3,			-2, .75, -.3,
	-2.7, 1.65, 0,		-2.7, 1.425, 0,				-2.5, .975,	0,				-2, .75, 0,

	1.7, 1.275, 0,		2.6, 1.275, 0,				2.3, 1.95, 0,				2.7, 2.25, 0,
	1.7, 1.275, .66,	2.6, 1.275, .66,			2.3, 1.95,	.25,			2.7, 2.25, .25,
	1.7, .45, .66,		3.1, .675, .66,				2.4, 1.875,	.25,			3.3, 2.25, .25,
	1.7, .45, 0,		3.1, .675, 0,				2.4, 1.875,	0,				3.3, 2.25, 0,

	1.7, .45, 0,		3.1, .675, 0,				2.4, 1.875,	0,				3.3, 2.25, 0,
	1.7, .45, -.66,		3.1, .675, -.66,			2.4, 1.875,	-.25,			3.3, 2.25, -.25,
	1.7, 1.275, -.66,	2.6, 1.275, -.66,			2.3, 1.95, -.25,			2.7, 2.25, -.25,
	1.7, 1.275, 0,		2.6, 1.275, 0,				2.3, 1.95, 0,				2.7, 2.25, 0,

	2.7, 2.25, 0,		2.8, 2.325, 0,				2.9, 2.325, 0,				 2.8, 2.25, 0,
	2.7, 2.25, .25,		2.8, 2.325, .25,			2.9, 2.325,	.15,			2.8, 2.25, .15,
	3.3, 2.25, .25,		3.525, 2.34375, .25,		3.45, 2.3625, .15,			3.2, 2.25, .15,
	3.3, 2.25, 0,		3.525, 2.34375, 0,			3.45, 2.3625, 0,			3.2, 2.25, 0,

	3.3, 2.25, 0,		3.525, 2.34375, 0,			3.45, 2.3625, 0,			3.2, 2.25, 0,
	3.3, 2.25, -.25,	3.525, 2.34375, -.25,		3.45, 2.3625, -.15,			3.2, 2.25, -.15,
	2.7, 2.25, -.25,	2.8, 2.325, -.25,			2.9, 2.325, -.15,			2.8, 2.25, -.15,
	2.7, 2.25, 0,		2.8, 2.325, 0,				2.9, 2.325,	0,				2.8, 2.25, 0,

	0, 3, 0,			.8, 3, 0,					0, 2.7, 0,					.2, 2.55, 0,
	0, 3, .002,			.8,	3, .45,					0, 2.7,	0,					.2, 2.55, .112,
	.002, 3, 0,			.45, 3, .8,					0, 2.7,	0,					.112, 2.55, .2,
	0, 3, 0,			0, 3, .8,					0, 2.7,	0,					0, 2.55, .2,

	0, 3, 0,			0, 3, .8,					0, 2.7,	0,					0, 2.55, .2,
	-.002, 3, 0,		-.45, 3, .8,				0, 2.7,	0,					-.112, 2.55, .2,
	0, 3, .002,			-.8, 3, .45,				0, 2.7,	0,					-.2, 2.55, .112,
	0, 3, 0,			-.8, 3, 0,					0, 2.7,	0,					-.2, 2.55, 0,

	0, 3, 0,			-.8, 3, 0,					0, 2.7,	0,					-.2, 2.55, 0,
	0, 3, -.002,		-.8, 3, -.45,				0, 2.7,	0,					-.2, 2.55, -.112,
	-.002, 3, 0,		-.45, 3, -.8,				0, 2.7,	0,					-.112, 2.55, -.2,
	0, 3, 0,			0, 3, -.8,					0, 2.7,	0,					0, 2.55, -.2,

	0, 3, 0,			0, 3, -.8,					0, 2.7,	0,					0, 2.55, -.2,
	.002, 3, 0,			.45, 3, -.8,				0, 2.7,	0,					.112, 2.55, -.2,
	0, 3, -.002,		.8,	3, -.45,				0, 2.7, 0,					.2, 2.55, -.112,
	0, 3, 0,			.8, 3, 0,					0, 2.7,	0,					.2, 2.55, 0,

	.2, 2.55, 0,		.4, 2.4, 0,					1.3, 2.4, 0,				1.3, 2.25, 0,
	.2, 2.55, .112,		.4, 2.4, .224,				1.3, 2.4, .728,				1.3, 2.25, .728,
	.112, 2.55, .2,		.224, 2.4, .4,				.728, 2.4, 1.3,				.728, 2.25, 1.3,
	0, 2.55, .2,		0, 2.4, .4,					0, 2.4, 1.3,				0, 2.25, 1.3,

	0, 2.55, .2,		0, 2.4, .4,					0, 2.4,	1.3,				0, 2.25, 1.3,
	-.112, 2.55, .2,	-.224, 2.4, .4,				-.728, 2.4, 1.3,			-.728, 2.25, 1.3,
	-.2, 2.55, .112,	-.4, 2.4, .224,				-1.3, 2.4, .728,			-1.3, 2.25, .728,
	-.2, 2.55, 0,		-.4, 2.4, 0,				-1.3, 2.4, 0,				-1.3, 2.25, 0,

	-.2, 2.55, 0,		-.4, 2.4, 0,				-1.3, 2.4, 0,				-1.3, 2.25, 0,
	-.2, 2.55, -.112,	-.4, 2.4, -.224,			-1.3, 2.4, -.728,			-1.3, 2.25, -.728,
	-.112, 2.55, -.2,	-.224, 2.4, -.4,			-.728, 2.4,	-1.3,			-.728, 2.25, -1.3,
	0, 2.55, -.2,		0, 2.4, -.4,				0, 2.4, -1.3,				0, 2.25, -1.3,

	0, 2.55, -.2,		0, 2.4, -.4,				0, 2.4, -1.3,				0, 2.25, -1.3,
	.112, 2.55, -.2,	.224, 2.4, -.4,				.728, 2.4, -1.3,			.728, 2.25, -1.3,
	.2, 2.55, -.112,	.4, 2.4, -.224,				1.3, 2.4, -.728,			1.3, 2.25, -.728,
	.2, 2.55, 0,		.4, 2.4, 0,					1.3, 2.4, 0,				1.3, 2.25, 0
};

B_Spline* teapot;

void generateTeapot()
{
	Shader shader;
	shader.shaderPointer = shaderProgram;
	shader.uMPMat = uMPMat;
	shader.uMPVMat = uMPVMat;
	shader.uColor = uColor;
	shader.uCamPos = uCamPos;
	
	teapot = new B_Spline(shader, 28);

	for (int i = 0; i < 28; ++i)
	{
		int k = i * 48;

		teapot->SetControlPoints(i,
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),

			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),

			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),

			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]),
			glm::vec3(teapotControlPoints[k++], teapotControlPoints[k++], teapotControlPoints[k++]));
	}

	teapot->transform().position = glm::vec3(0.0f, -1.5f, 0.0f);
}

void SetupLights()
{
	Shader shader;
	shader.shaderPointer = shaderProgram;
	shader.uMPMat = uMPMat;
	shader.uMPVMat = uMPVMat;
	shader.uColor = uColor;
	shader.uCamPos = uCamPos;

	LightingManager::Init(shader);

	Light& light0 = LightingManager::GetLight(0);
	light0.angularVelocity = glm::angleAxis(30.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	light0.rotationOrigin = glm::vec3(-10.0f, 1.5f, 0.0f);
	light0.position = glm::vec3(0.0f, -0.0f, 0.0f);
	light0.color = glm::vec4(0.8f, 0.0f, 0.0f, 1.0f);
	light0.ambient = glm::vec4(0.4f, 0.0f, 0.0f, 1.0f);
	light0.power = 0.5f;
	light0.active = true;

	Light& light1 = LightingManager::GetLight(1);
	light1.position = glm::vec3(5.0f, 0.0f, 0.0f);
	light1.color = glm::vec4(0.0f, 0.8f, 0.0f, 1.0f);
	light1.ambient = glm::vec4(0.0f, 0.4f, 0.0f, 1.0f);
	light1.power = 0.5f;
	light1.active = true;

	Light& light2 = LightingManager::GetLight(2);
	light2.position = glm::vec3(-5.0f, 0.0f, 0.0f);
	light2.color = glm::vec4(0.0f, 0.0f, 0.8f, 1.0f);
	light2.ambient = glm::vec4(0.0f, 0.0f, 0.4f, 1.0f);
	light2.power = 0.5f;
	light2.active = true;

	Shader selfIllum;
	selfIllum.shaderPointer = selfIllumProgram;
	selfIllum.uMPVMat = uMPVMat_SI;
	selfIllum.uColor = uColor_SI;

	glGenBuffers(1, &cubeVAO);
	glBindVertexArray(cubeVAO);
	
	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_VERTEX_ARRAY, cubeVBO);
	glBufferData(cubeVBO, sizeof(vertices), &vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &cubeEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
	glBufferData(cubeEBO, sizeof(elements), &elements, GL_STATIC_DRAW);

	GLint posAttrib = glGetAttribLocation(selfIllum.shaderPointer, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

	//LightingManager::SetLightShape(new RenderShape(cubeVAO, Self_Lit, 36, GL_TRIANGLES, selfIllum), 0);
	RenderManager::AddShape(new RenderShape(cubeVAO, Self_Lit, 36, GL_TRIANGLES, selfIllum));
	//LightingManager::SetLightShape(new RenderShape(cubeVAO, Self_Lit, 36, GL_TRIANGLES, selfIllum), 1);
}

void initShaders()
{
	char* shaders[] = { "fshader.glsl", "vshader.glsl" };
	GLenum types[] = { GL_FRAGMENT_SHADER, GL_VERTEX_SHADER };
	int numShaders = 2;
	
	shaderProgram = initShaders(shaders, types, numShaders);

	uMPMat = glGetUniformLocation(shaderProgram, "mpMat");
	uMPVMat = glGetUniformLocation(shaderProgram, "mpvMat");
	uColor = glGetUniformLocation(shaderProgram, "color");
	uCamPos = glGetUniformLocation(shaderProgram, "camPos");

	char* si_Shaders[] = { "self_illum_vert.glsl", "self_illum_frag.glsl" };
	GLenum si_Types[] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
	
	selfIllumProgram = initShaders(si_Shaders, si_Types, numShaders);

	uMPVMat_SI = glGetUniformLocation(selfIllumProgram, "mpvMat");
	uColor_SI = glGetUniformLocation(selfIllumProgram, "color");
}

void init()
{
	if (!glfwInit()) exit(EXIT_FAILURE);

	//Create window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	window = glfwCreateWindow(800, 600, "Geometric_Lighting_Improved-GLFW", NULL, NULL); // Windowed

	//Activate window
	glfwMakeContextCurrent(window);

	glewExperimental = true;
	glewInit();

	initShaders();

	glfwSetTime(0.0);

	time_t timer;
	time(&timer);
	srand((unsigned int)timer);

	//generateTeapot();

	SetupLights();

	InputManager::Init(window);
	CameraManager::Init(800.0f / 600.0f, 60.0f, 0.1f, 100.0f);
}

void step()
{
	// Clear to black
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	InputManager::Update();

	// Get delta time since the last frame
	float dt = glfwGetTime();
	glfwSetTime(0.0);

	float dTheta = 45.0f * InputManager::rightKey();
	dTheta -= 45.0f * InputManager::leftKey();

	//teapot->transform().angularVelocity = glm::angleAxis(dTheta, glm::vec3(0.0f, 1.0f, 0.0f));

	CameraManager::Update(dt);

	RenderManager::Update(dt);

	//teapot->Update(dt);

	LightingManager::Update(dt);

	RenderManager::Draw();

	// Swap buffers
	glfwSwapBuffers(window);
}

void cleanUp()
{
	glDeleteProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	RenderManager::DumpData();

	delete teapot;

	glfwTerminate();
}

int main()
{
	init();

	while (!glfwWindowShouldClose(window))
	{
		step();
		glfwPollEvents();
	}

	cleanUp();

	return 0;
}
