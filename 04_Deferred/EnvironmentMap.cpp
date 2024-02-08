#include "EnvironmentMap.h"
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <iostream>
#include "Entity.h"

ProgramObject* EnvironmentMap::shader = nullptr;

std::array<glm::mat4, 6> getTransform(const glm::vec3& center, const float& radius) {
	constexpr float aspect = 1.f;

	std::array<glm::mat4, 6> transforms;
	glm::mat4 perspective = glm::perspective(glm::radians(90.0f), aspect, std::min(0.5f,radius), 1000.f);

	transforms[0] = perspective * glm::lookAt(center, center + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
	transforms[1] = perspective * glm::lookAt(center, center + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
	transforms[2] = perspective * glm::lookAt(center, center + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
	transforms[3] = perspective * glm::lookAt(center, center + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
	transforms[4] = perspective * glm::lookAt(center, center + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
	transforms[5] = perspective * glm::lookAt(center, center + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));

	return transforms;
}

EnvironmentMap::EnvironmentMap(glm::vec3 center, float radius) : resolution(480), frequency(10.f), refreshTime(0.0f) {
	if (shader == nullptr) {
		shader = new ProgramObject();
		shader->Init({
			{ GL_VERTEX_SHADER,		"Shaders/environment.vert" },
			{ GL_GEOMETRY_SHADER,	"Shaders/environment.geom" },
			{ GL_FRAGMENT_SHADER,	"Shaders/environment.frag" }
		});
	}
	transforms = getTransform(center, radius);
	createFrameBuffer(resolution);
}

EnvironmentMap::~EnvironmentMap() {
	glDeleteTextures(1, &cubemap);
	glDeleteTextures(1, &cubemapDepth);
	glDeleteFramebuffers(1, &sceneFBO);
}

void EnvironmentMap::UpdateScene(const std::vector<Entity>& entities) {
	bool update = (int)(refreshTime + 6.0f / frequency) > (int)refreshTime;
	int lower = (int)refreshTime;
	refreshTime += 6.0f / frequency;
	int upper = (int)refreshTime % 6;

	refreshTime = std::fmod(refreshTime, 6.0f);
	if (!update) return;

	std::array<int, 6> updateValues;
	updateValues.fill(0);
	if (lower < upper) {
		for (int i = lower; i < upper; ++i)updateValues[i] = 1;
		clearTexture(lower, upper);
	}
	else if (lower == upper) {
		updateValues.fill(1);
		clearTexture(0, 6);
	}
	else {
		for (int i = lower; i < 6; ++i)updateValues[i] = 1;
		for (int i = 0; i < upper; ++i)updateValues[i] = 1;
		clearTexture(lower, 6);
		clearTexture(0, upper);
	}

	glViewport(0, 0, resolution, resolution);

	glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
	shader->Use();

	glUniformMatrix4fv(shader->GetLocation("transforms"), 6, GL_FALSE, (float*)transforms.data());
	glUniform1iv(shader->GetLocation("update"), 6, updateValues.data());
	for (const auto& entity : entities) {
		if (entity.reflected) {
			entity.setTexture(*shader);
			shader->SetUniform("world", entity.getLocalModelMatrix());
			entity.mesh->draw();
		}
	}

	shader->Unuse();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void EnvironmentMap::clearTexture(int lower, int upper) {
	if (upper == 0) return;
	static glm::vec3 clearValue = glm::vec3(0.0f);
	static float clearValueDepth = 1.0f;

	glClearTexSubImage(cubemap, 0, 0, 0, lower, resolution, resolution,
		upper - lower, GL_RGB, GL_FLOAT, &clearValue);

	glClearTexSubImage(cubemapDepth, 0, 0, 0, lower, resolution, resolution,
		upper - lower, GL_DEPTH_COMPONENT, GL_FLOAT, &clearValueDepth);
}

GLuint EnvironmentMap::getTexture() const {
	return cubemap;
}

void EnvironmentMap::createFrameBuffer(int resolution) {
	if (frameBufferCreated) {
		glDeleteTextures(1, &cubemap);
		glDeleteTextures(1, &cubemapDepth);
		glDeleteFramebuffers(1, &sceneFBO);
	}

	glGenTextures(1, &cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8,
			resolution, resolution, 0, GL_RGB, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glCreateFramebuffers(1, &sceneFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, cubemap, 0);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "Error creating depth attachment" << std::endl;
		exit(1);
	}

	glGenTextures(1, &cubemapDepth);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapDepth);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24,
			resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubemapDepth, 0);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "Error creating depth attachment" << std::endl;
		exit(1);
	}

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Incomplete framebuffer (";
		switch (status) {
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:			std::cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";		break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:	std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
		case GL_FRAMEBUFFER_UNSUPPORTED:					std::cout << "GL_FRAMEBUFFER_UNSUPPORTED";					break;
		}
		std::cout << ")" << std::endl;
		exit(1);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	frameBufferCreated = true;
}

void EnvironmentMap::updatePosition(glm::vec3 center, float radius) {
	transforms = getTransform(center, radius);
}