#ifndef SSAO__H
#define SSAO__H

#include <GL/glew.h>
#include <glm/common.hpp>

#include "Includes/ProgramObject.h"
#include "Includes/gCamera.h"

class SSAO {
private:
	bool frameBuffersCreated{ false };
	GLuint ssaoFrameBuffer;
	GLuint ssaoTexture;
	GLuint noise_texture;

	ProgramObject ssao_program;

public:
	SSAO();
	~SSAO();

	void renderSSAO(const GLuint normalBuffer, const GLuint depthBuffer, const gCamera& camera);
	void createFrameBuffer(int width, int height);
	GLuint getSSAO();

};

#endif // SSAO__H
