/*
* Copyright (c) 2015 Owen Glofcheski
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source
*    distribution.
*/

#ifndef _GL_HPP_
#define _GL_HPP_

#include <iostream>
#include <unordered_map>

#include <GL\glew.h>

#include "util.hpp"

#define GL_CHECK() PrintGLError(__FILE__, __LINE__)

int PrintGLError(char *file, int line);

typedef GLuint Texture;
typedef GLint Uniform;

struct Shader
{
	GLuint program = 0;
};

void glPrintProgramInfo(GLuint program);
GLuint glLoadShader(const std::string &vs_filename, const std::string &fs_filename);
void glUseShader(const Shader &shader);
Uniform glGetUniform(const Shader &shader, const std::string &name);

void glDrawRect(float left, float right, float bottom, float top, float depth);
void glDrawTexturedQuad(GLuint texture);

void glCreateTexture1D(Texture *texture, int width, int channels, void *data);
void glCreateTexture2D(Texture *texture, int width, int height, int channels, void *data);
void glCreateTexture3D(Texture *texture, int width, int height, int depth, int channels, void *data);

void glCreateDepthTexture(Texture *texture);

void glSetPerspectiveProjection(const GLdouble fov, const GLdouble aspect, const GLdouble n, const GLdouble f);
void glSetPerspectiveProjection(const GLdouble fov, const size_t width, 
	const size_t height, const GLdouble n, const GLdouble f);
void glSetOrthographicProjection(const GLdouble left, const GLdouble right, const GLdouble bottom,
	const GLdouble top, const GLdouble n, const GLdouble f);

#endif