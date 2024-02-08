#pragma once

// C++ includes
#include <memory>

// GLEW
#include <GL/glew.h>

// SDL
#include <SDL.h>
#include <SDL_opengl.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

#include "Includes/gCamera.h"

#include "Includes/ProgramObject.h"
#include "Includes/BufferObject.h"
#include "Includes/VertexArrayObject.h"
#include "Includes/TextureObject.h"

#include "Includes/Mesh_OGL3.h"

#include "Lights.h"
#include "SSAO.h"
#include "Entity.h"

class CMyApp
{
public:
	CMyApp(void);
	~CMyApp(void);

	bool Init();
	void Clean();

	void Update();
	void Render();

	void KeyboardDown(SDL_KeyboardEvent&);
	void KeyboardUp(SDL_KeyboardEvent&);
	void MouseMove(SDL_MouseMotionEvent&);
	void MouseDown(SDL_MouseButtonEvent&);
	void MouseUp(SDL_MouseButtonEvent&);
	void MouseWheel(SDL_MouseWheelEvent&);
	void Resize(int, int);

	float getHeight(float x, float z);
	glm::vec4 getNormalHeight(float x, float z);
protected:
	void CreateFrameBuffer(int, int);
	void DrawScene(const glm::mat4&, const glm::mat4&, const bool);
	void renderLightGUI(LightType);
	void renderEntityGUI();


	ProgramObject		m_program;
	ProgramObject       m_programPostprocess;
	ProgramObject		nonReflective;
	ProgramObject		reflective;

	gCamera				m_camera;

	std::vector<Entity> entities;

	bool frameBufferCreated{ false };
	GLuint sceneFrameBuffer;
	GLuint diffuseTexture;
	GLuint normalTextureView;
	GLuint depthTexturePers;

	Lights lights;
	SSAO ssao;

	const std::array<glm::vec3, 16> landscapePoints = {
		glm::vec3(-200, 20, -200), glm::vec3(-200, 0, -100), glm::vec3(-200,  -10, 0), glm::vec3(-200, 20, 100),
		glm::vec3(-100, 30, -200), glm::vec3(-100, 40, -100), glm::vec3(-100,  0, 0), glm::vec3(-100, 0, 100),
		glm::vec3(0, 9, -200), glm::vec3(0, 40, -100), glm::vec3(0,  30, 0), glm::vec3(0, 0, 100),
		glm::vec3(100, 13, -200), glm::vec3(100, 2, -100), glm::vec3(100, -2, 0), glm::vec3(100, 0, 100)
	};
};

