#ifndef _TEXT_HPP_
#define _TEXT_HPP_

#include <vector>
#include <cstring>
#include <GL/glew.h>

#include "algebra.hpp"
#include "gl.hpp"

extern GLuint kFontTexture;
extern GLuint kTextVertexBuffer;
extern GLuint kTextUVBuffer;
extern GLuint kTextShader;
extern GLuint kTextUniform;

inline
void initText() {
	//kFontTexture = 

	glGenBuffers(1, &kTextVertexBuffer);
	glGenBuffers(1, &kTextUVBuffer);

	kTextShader = glLoadShader("text.vert", "text.frag");
	kTextUniform = glGetUniformLocation(kTextShader, "uFontTexture");
}

inline
void printText2D(const char * text, int x, int y, int size) {
	std::vector<Vector2> vertices;
	std::vector<Vector2> UVs;
	for (unsigned int i = 0; i < strlen(text); ++i) {
		// Create triangles for character
		Vector2 vertex_up_left = Vector2(x + i*size, y + size);
		Vector2 vertex_up_right = Vector2(x + i*size + size, y + size);
		Vector2 vertex_down_right = Vector2(x + i*size + size, y);
		Vector2 vertex_down_left = Vector2(x + i*size, y);

		// Triangle 1
		vertices.push_back(vertex_up_left);
		vertices.push_back(vertex_down_left);
		vertices.push_back(vertex_up_right);

		// Triangle 2
		vertices.push_back(vertex_down_right);
		vertices.push_back(vertex_up_right);
		vertices.push_back(vertex_down_left);

		// Retrieve uv coords for font texture
		char character = text[i];
		float uv_x = (character % 16) / 16.0f;
		float uv_y = (character / 16) / 16.0f;

		Vector2 uv_up_left = Vector2(uv_x, uv_y);
		Vector2 uv_up_right = Vector2(uv_x + 1.0f / 16.0f, uv_y);
		Vector2 uv_down_right = Vector2(uv_x + 1.0f / 16.0f, (uv_y + 1.0f / 16.0f));
		Vector2 uv_down_left = Vector2(uv_x, (uv_y + 1.0f / 16.0f));
		UVs.push_back(uv_up_left);
		UVs.push_back(uv_down_left);
		UVs.push_back(uv_up_right);

		UVs.push_back(uv_down_right);
		UVs.push_back(uv_up_right);
		UVs.push_back(uv_down_left);
	}
	glBindBuffer(GL_ARRAY_BUFFER, kTextVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vector2), &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, kTextUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(Vector2), &UVs[0], GL_STATIC_DRAW);

	// Bind shader
	glUseProgram(kTextShader);

	// Bind texture
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, Text2DTextureID);
	// Set our "myTextureSampler" sampler to user Texture Unit 0
	glUniform1i(kTextUniform, 0);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, kTextVertexBuffer);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, kTextUVBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	glDisable(GL_BLEND);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

#endif