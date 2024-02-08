#ifndef ENVIRONMENT_MAP__H
#define ENVIRONMENT_MAP__H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Includes/Mesh_OGL3.h"
#include "Includes/ProgramObject.h"

class Entity;

class EnvironmentMap {
private:
	static ProgramObject* shader;

	GLuint sceneFBO;
	GLuint cubemap;
	GLuint cubemapDepth;
	bool frameBufferCreated{ false };

	std::array<glm::mat4, 6> transforms;
	float refreshTime;


	void clearTexture(int lower, int upper);
public:
	int resolution;
	float frequency;

	EnvironmentMap(glm::vec3 center, float radius);
	~EnvironmentMap();
	void UpdateScene(const std::vector<Entity>& entities);
	void createFrameBuffer(int resolution);
	void updatePosition(glm::vec3 center, float radius);
	GLuint getTexture() const;
};

#endif // !ENVIRONMENT_MAP__H
