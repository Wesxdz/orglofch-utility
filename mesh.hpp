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

#ifndef _MESH_HPP_
#define _MESH_HPP_

#include "gl.hpp"

// TODO(orglofch): Store vertices and faces so we can perform collision detection as well as rendering
struct Mesh
{
	Mesh() : vertexVBO(0), indexVBO(0), face_count(0) {}

	GLuint vertexVBO;
	GLuint indexVBO;
	unsigned int face_count;
};

Mesh LoadOBJ(const std::string &filename);
void RenderMesh(const Mesh &mesh);

#endif