#include "RenderShape.h"
#include "CameraManager.h"

RenderShape::RenderShape(GLint vao, GLsizei count, GLenum mode, Shader shader, glm::vec4 color)
{
	_vao = vao;
	_count = count;
	_mode = mode;
	_shader = shader;
	_color = color;
	_currentColor = color;

	_transform = Transform();
}
RenderShape::~RenderShape()
{

}

void RenderShape::Update(float dt)
{
	// Update values
	_transform.position += _transform.linearVelocity * dt;
	_transform.rotation = glm::slerp(_transform.rotation, _transform.rotation * _transform.angularVelocity, dt);

	// Apply transforms
	glm::mat4 translateMat = glm::translate(glm::mat4(), _transform.position);

	glm::mat4 rotateOriginMat = glm::translate(glm::mat4(), _transform.rotationOrigin);
	glm::mat4 rotateMat = rotateOriginMat * glm::mat4_cast(_transform.rotation) * glm::inverse(rotateOriginMat);

	glm::mat4 scaleOriginMat = glm::translate(glm::mat4(), _transform.scaleOrigin);
	glm::mat4 scaleMat = scaleOriginMat * glm::scale(glm::mat4(), _transform.scale) * glm::inverse(scaleOriginMat);

	glm::mat4 *parentModelMat = _transform.parent ? &_transform.parent->modelMat : &glm::mat4();

	_transform.modelMat = (*parentModelMat) * (translateMat * scaleMat* rotateMat);

	_currentColor = _color;
}
void RenderShape::Draw()
{
	if (_active)
	{
		glBindVertexArray(_vao);

		glUseProgram(_shader.shaderPointer);

		glm::mat4 modelMat = _transform.modelMat;
		glm::mat4 viewMat = CameraManager::ViewMat();
		glm::mat4 projMat = CameraManager::ProjMat();
		glm::vec4 camPos = CameraManager::CamPos();

		glUniformMatrix4fv(_shader.uModelMat, 1, GL_FALSE, glm::value_ptr(modelMat));
		glUniformMatrix4fv(_shader.uViewMat, 1, GL_FALSE, glm::value_ptr(viewMat));
		glUniformMatrix4fv(_shader.uProjMat, 1, GL_FALSE, glm::value_ptr(projMat));
		glUniform4fv(_shader.uColor, 1, glm::value_ptr(_currentColor));
		glUniform4fv(_shader.uCamPos, 1, glm::value_ptr(camPos));

		//Make draw call
		glDrawElements(_mode, _count, GL_UNSIGNED_INT, 0);
	}
}

const glm::vec4& RenderShape::color()
{
	return _color;
}
glm::vec4& RenderShape::currentColor()
{
	return _currentColor;
}
Transform& RenderShape::transform()
{
	return _transform;
}
GLint RenderShape::vao()
{
	return _vao;
}
GLsizei RenderShape::count()
{
	return _count;
}
void RenderShape::count(GLsizei newSize)
{
	_count = newSize;
}
GLenum RenderShape::mode()
{
	return _mode;
}
Shader RenderShape::shader()
{
	return _shader;
}

bool& RenderShape::active()
{
	return _active;
}
