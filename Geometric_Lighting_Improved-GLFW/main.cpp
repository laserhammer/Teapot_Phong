/*
Geometric Lighting Improved
(c) 2015
original authors: Benjamin Robbins
Written under the supervision of David I. Schwartz, Ph.D., and
supported by a professional development seed grant from the B. Thomas
Golisano College of Computing & Information Sciences
(https://www.rit.edu/gccis) at the Rochester Institute of Technology.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*	This example makes use of the teapot bezier spline established in the B-Spline example and thus retains many of the same components.
*	This version builds upon the geometric lighting example by adding a lighting manager. This allows for multiple dynamic lights
*	to be placed in the scene. Furthermore, it adds a phong lighting model to the fragment shader, making our teapot even prettier.
*	Also, to make things a bit easier to see and understand, this code includes some boxes using self-lit shaders to indicate the positions
*	of the lights in the scene.
*	There are 4 static component classes that make up the base functionality of this program.
*
*	1) RenderManager
*	- This class maintains the display list for the scene being rendered and thus handles the processes of updating and drawing all
*	of the RenderShapes that have been instantiated in the scene.
*
*	2) CameraManager
*	- This class maintains data relating to the view and projection matrices used in the rendering pipeline. It also handles updating
*	this data based on user input.
*
*	3) InputManager
*	- This class maintains data for the current state of user input for the mouse and keyboard.
*
*	4) LightingManager
*	- This class maintains data for an array of eight lights, each posessing a trasform, color and power, handles the updating thereof and 
*	maintains gpu-side buffers reflecting this data for use in the shaders. 
*
*	B_Spline
*	- This non-static class is instantiated to maintain an array of Patch objects. Control point data is sent to this class to manipulate
*	component patches.
*
*	Patch
*	- This non-static class handles the data storage and updating for a single bezier surface containing a dynamically drawn RenderShape
*	and 16 control points that through a third-order Bernstein polynomial determine mathematically the positions of the vertices comprising
*	the surface.
*
*	RenderShape
*	- This class tracks instance data for every shape that is drawn to the screen. This data primarily includes a vertex array object and
*	transform data. This transform data is used to generate the model matrix used along with the view and projection matrices in the
*	rendering pipeline.
*
*	Init_Shader
*	- Contains static functions for reading, compiling and linking shaders.
*
*
*	SHADERS
*
*	vShader.glsl
*	- Simple through shader, applies transforms to verts and normals before passing them through to the fragment shader.
*
*	fShader.glsl
*	- Uses a hard-coded point-light to apply the color of the light to the current fragment based on the phong lighting model.
*	see: http://en.wikipedia.org/wiki/Phong_reflection_model
*	Under the phong lighting model, the color of a surface is dertermined by ls + ld + la
*	la is the ambient light of the scene and is a set constant value.
*	ld is the diffuse light determined by the lambert model ld = clamp(Normal dot lightDir, 0, 1) * diffuseColor * diffusePower / distance^2
*	ls is the specular color determined by the angle between the reflection of the light vector about the normal of the surface and the 
*	viewing vector of the camera. As with lambert's law, the dot product of the viewing vector and the reflection vector is equal to the cosine
*	of the angle between them. Then get rid of any values less than 0. Now obviously, not all materials are equally shiny. smooth surfaces will
*	have small, intense highlights while rough surfaces will have larger highlights that fall off in intensity more slowly. Any of this spectrum
*	of effects  can be accomplished by raising this highlight value to a shininess exponent. so highlightValue^Shininess. Higher the exponent,
*	the small and more intense your specular highlights will appear, or conversly lower exponents will create larger and duller highlights. 
*	Approach an exponent of 0.0 for some seriously trippy highlight action.
*	So ls = pow(max(reflect(-lightDr, Normal) dot viewingVector, 0.0), shininessExponent) * specularColor
*
*	self_illum_vert.glsl
*	- Through shader
*
*	self_illum_frag.glsl
*	- Renders tris using only the color passed in, ignoring lighting data
*/
#include <GLEW\GL\glew.h>
#include <GLFW\glfw3.h>
#include <GLM\gtc\type_ptr.hpp>
#include <GLM\gtc\matrix_transform.hpp>
#include <GLM\gtc\quaternion.hpp>
#include <GLM\gtc\random.hpp>
#include <iostream>
#include <ctime>

#include "RenderShape.h"
#include "Init_Shader.h"
#include "RenderManager.h"
#include "InputManager.h"
#include "B-Spline.h"
#include "Patch.h"
#include "CameraManager.h"
#include "LightingManager.h"

GLFWwindow* window;

Shader phongShader;

Shader selfIllumShader;

GLuint cubeVAO;
GLuint cubeVBO;
GLuint cubeEBO;

B_Spline* teapot;
Light* lights[3];

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
#pragma region Teapot Control Points
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
#pragma endregion 
};

void generateTeapot()
{
	teapot = new B_Spline(phongShader, 28);

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
	LightingManager::Init(phongShader);

	lights[0] = &LightingManager::GetLight(0);
	lights[0]->angularVelocity = glm::angleAxis(30.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	lights[0]->rotationOrigin = glm::vec3(-3.0f, 1.5f, 0.0f);
	lights[0]->position = glm::vec3(3.0f, -1.5f, 0.0f);
	lights[0]->color = glm::vec4(0.8f, 0.0f, 0.0f, 1.0f);
	lights[0]->power = 5.0f;
	
	lights[1] = &LightingManager::GetLight(1);
	lights[1]->position = glm::vec3(3.0f, 1.0f, 0.0f);
	lights[1]->color = glm::vec4(0.0f, 0.8f, 0.0f, 1.0f);
	lights[1]->power = 5.0f;

	lights[2] = &LightingManager::GetLight(2);
	lights[2]->position = glm::vec3(-3.0f, 1.0f, 0.0f);
	lights[2]->color = glm::vec4(0.0f, 0.0f, 0.8f, 1.0f);
	lights[2]->power = 5.0f;

	LightingManager::SetAmbient(glm::vec3(0.5f, 0.5f, 0.5f));

	// Generate the buffer for the cubes used to show the lights' positions
	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);
	
	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &cubeEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), &elements, GL_STATIC_DRAW);

	GLint posAttrib = glGetAttribLocation(selfIllumShader.shaderPointer, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

	LightingManager::SetLightShape(new RenderShape(cubeVAO, 36, GL_TRIANGLES, selfIllumShader), 0);
	LightingManager::SetLightShape(new RenderShape(cubeVAO, 36, GL_TRIANGLES, selfIllumShader), 1);
	LightingManager::SetLightShape(new RenderShape(cubeVAO, 36, GL_TRIANGLES, selfIllumShader), 2);
}

void initShaders()
{
	char* shaders[] = { "fshader.glsl", "vshader.glsl" };
	GLenum types[] = { GL_FRAGMENT_SHADER, GL_VERTEX_SHADER };
	
	GLuint phongShaderProgram = initShaders(shaders, types, 2);

	phongShader = Shader();
	phongShader.shaderPointer = phongShaderProgram;
	phongShader.uModelMat = glGetUniformLocation(phongShaderProgram, "modelMat");
	phongShader.uViewMat = glGetUniformLocation(phongShaderProgram, "viewMat");
	phongShader.uProjMat = glGetUniformLocation(phongShaderProgram, "projMat");
	phongShader.uColor = glGetUniformLocation(phongShaderProgram, "color");
	phongShader.uCamPos = glGetUniformLocation(phongShaderProgram, "camPos");

	char* si_Shaders[] = { "self_illum_vert.glsl", "self_illum_frag.glsl" };
	GLenum si_Types[] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
	
	GLuint selfIllumProgram = initShaders(si_Shaders, si_Types, 2);

	selfIllumShader = Shader();

	selfIllumShader.shaderPointer = selfIllumProgram;
	selfIllumShader.uModelMat = glGetUniformLocation(selfIllumProgram, "modelMat");
	selfIllumShader.uViewMat = glGetUniformLocation(selfIllumProgram, "viewMat");
	selfIllumShader.uProjMat = glGetUniformLocation(selfIllumProgram, "projMat");
	selfIllumShader.uColor = glGetUniformLocation(selfIllumProgram, "color");
	selfIllumShader.uCamPos = -1;
}

void init()
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
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

	SetupLights();

	generateTeapot();

	InputManager::Init(window);
	CameraManager::Init(800.0f / 600.0f, 60.0f, 0.1f, 100.0f);

	glEnable(GL_DEPTH_TEST);
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

	teapot->transform().angularVelocity = glm::angleAxis(dTheta, glm::vec3(0.0f, 1.0f, 0.0f));

	CameraManager::Update(dt);

	RenderManager::Update(dt);

	LightingManager::Update(dt);

	teapot->Update(dt);

	RenderManager::Draw();

	// Swap buffers
	glfwSwapBuffers(window);
}

void cleanUp()
{
	glDeleteProgram(phongShader.shaderPointer);
	glDeleteProgram(selfIllumShader.shaderPointer);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &cubeEBO);
	glDeleteVertexArrays(1, &cubeVAO);

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
