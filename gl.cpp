#include "gl.hpp"

std::unordered_map<std::string, GLuint> loaded_shaders;

int PrintGLError(char *file, int line) {
	GLenum glErr;
	int    retCode = 0;

	glErr = glGetError();
	if (glErr != GL_NO_ERROR) {
		printf("glError in file %s @ line %d: %s\n",
			file, line, gluErrorString(glErr));
		retCode = 1;
	}
	return retCode;
}

GLuint glLoadShader(const std::string &vs_filename, const std::string &fs_filename) {
	// Try to retrieve the shader from the already loaded shaders
	std::string shader_key = vs_filename + fs_filename;
	auto loaded_shader_itr = loaded_shaders.find(shader_key);
	if (loaded_shader_itr != loaded_shaders.end()) {
		return loaded_shader_itr->second;
	}

	// Load the complete shader
	std::string vertex_shader_string = ReadFile(vs_filename.c_str());
	std::string fragment_shader_string = ReadFile(fs_filename.c_str());
	int vlen = vertex_shader_string.length();
	int flen = fragment_shader_string.length();

	if (vertex_shader_string.empty())
		return false;
	if (fragment_shader_string.empty())
		return false;

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	const char *vertex_shader_cstr = vertex_shader_string.c_str();
	const char *fragment_shader_cstr = fragment_shader_string.c_str();
	glShaderSource(vertex_shader, 1, (const GLchar **)&vertex_shader_cstr, &vlen);
	glShaderSource(fragment_shader, 1, (const GLchar **)&fragment_shader_cstr, &flen);

	GLint compiled;

	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
	if (compiled == false) {
		printf("Vertex shader not compliled\n");
		glPrintProgramInfo(vertex_shader);

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		return false;
	}

	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
	if (compiled == false) {
		printf("Fragment shader not compiled\n");
		glPrintProgramInfo(fragment_shader);

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		return false;
	}

	GLuint shader = glCreateProgram();

	glAttachShader(shader, vertex_shader);
	glAttachShader(shader, fragment_shader);

	glLinkProgram(shader);

	GLint is_linked;
	glGetProgramiv(shader, GL_LINK_STATUS, (GLint *)&is_linked);
	if (is_linked == false) {
		printf("Failed to link shader\n");

		GLint max_length;
		glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &max_length);
		if (max_length > 0) {
			char *p_link_info_log = new char[max_length];
			glGetProgramInfoLog(shader, max_length, &max_length, p_link_info_log);
			printf("%s\n", p_link_info_log);
			delete[] p_link_info_log;
		}

		glDetachShader(shader, vertex_shader);
		glDetachShader(shader, fragment_shader);
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		glDeleteProgram(shader);
		return 0;
	}

	glDetachShader(shader, vertex_shader);
	glDetachShader(shader, fragment_shader);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	loaded_shaders[shader_key] = shader;
	return shader;
}

void glPrintProgramInfo(GLuint program) {
	int info_log_len = 0;
	int char_written = 0;
	GLchar *info_log;

	glGetShaderiv(program, GL_INFO_LOG_LENGTH, &info_log_len);

	if (info_log_len > 0) {
		info_log = new GLchar[info_log_len];
		glGetShaderInfoLog(program, info_log_len, &char_written, info_log);
		printf("Info: %s\n", info_log);
		delete[] info_log;
	}
}

void glUseShader(const Shader &shader) {
	glUseProgram(shader.program);
}

Uniform glGetUniform(const Shader &shader, const std::string &name) {
	return glGetUniformLocation(shader.program, name.c_str());
}

// TODO(orglofch): Possibly remove attributes and force use of transforms
void glDrawRect(float left, float right, float bottom, float top, float depth) {
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(left, bottom, depth);
	glTexCoord2f(0, 1); glVertex3f(left, top, depth);
	glTexCoord2f(1, 1); glVertex3f(right, top, depth);
	glTexCoord2f(1, 0); glVertex3f(right, bottom, depth);
	glEnd();
}

// TODO(orglofch): Remove probably
void glDrawTexturedQuad(GLuint texture) {
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, -1.0, -0.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(1.0, -1.0, -0.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(1.0, 1.0, -0.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(-1.0, 1.0, -0.0);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void glCreateTexture1D(Texture *texture, int width, int channels, void *data) {
	NOT_NULL(data);

	glEnable(GL_TEXTURE_1D);
	glGenTextures(1, texture);
	if (*texture < 1) {
		LOG("Failed to gen texture\n");
		return;
	}

	glBindTexture(GL_TEXTURE_1D, *texture);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage1D(GL_TEXTURE_1D, 0, channels == 4 ? GL_RGBA32F : GL_RGB32F,
		width, 0, channels == 4 ? GL_RGBA : GL_RGB, GL_FLOAT, data);
	glBindTexture(GL_TEXTURE_1D, 0);
}

void glCreateTexture2D(Texture *texture, int width, int height, int channels, void *data) {
	NOT_NULL(data);

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, texture);
	if (*texture < 1) {
		LOG("Failed to gen texture\n");
		return;
	}

	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, channels == 4 ? GL_RGBA32F : GL_RGB32F,
		width, height, 0, channels == 4 ? GL_RGBA : GL_RGB,
		GL_FLOAT, data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void glCreateTexture3D(Texture *texture, int width, int height, int depth, int channels, void *data) {
	NOT_NULL(data);

	glEnable(GL_TEXTURE_3D);
	glGenTextures(1, texture);
	if (*texture < 1) {
		LOG("Failed to gen texture\n");
		return;
	}

	glBindTexture(GL_TEXTURE_3D, *texture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage3D(GL_TEXTURE_3D, 0, channels == 4 ? GL_RGBA32F : GL_RGB32F,
		width, height, depth, 0, channels == 4 ? GL_RGBA : GL_RGB,
		GL_FLOAT, data);
	glBindTexture(GL_TEXTURE_3D, 0);
}

// TODO(orglofch): Possibly combine with 2D call above
void glCreateDepthTexture(Texture *texture) {
	glGenTextures(1, texture);
	if (*texture < 1) {
		LOG("Failed to gen depth texture\n");
		return;
	}

	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_3D, 0);
}

void glSetPerspectiveProjection(const GLdouble fov, const GLdouble aspect, const GLdouble n, const GLdouble f) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(fov, aspect, n, f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void glSetPerspectiveProjection(const GLdouble fov, const size_t width, 
		const size_t height, const GLdouble n, const GLdouble f) {
	glSetPerspectiveProjection(fov, 1.0 * width / height, n, f);
}

void glSetOrthographicProjection(const GLdouble left, const GLdouble right, 
		const GLdouble bottom, const GLdouble top, const GLdouble n, const GLdouble f) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(left, right, bottom, top, n, f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}