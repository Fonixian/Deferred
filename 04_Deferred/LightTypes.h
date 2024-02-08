#ifndef LIGHT_TYPES_HH
#define LIGHT_TYPES_HH

#include <vector>
#include <GL/glew.h>
#include "Includes/gCamera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

#include <iostream>

enum LightType;

struct LightInfo {
	glm::vec3 color;
	union {
		glm::vec3 position;
		glm::vec3 direction;
	};
	bool cast_shadow;
	int shadow_map_resolution_wh[2];
	float refreshFrequency;

	LightInfo(glm::vec3 color, glm::vec3 pos_dir, bool cast_shadow, std::vector<int> resolution_wh);
	LightInfo();
};

struct Light {
	glm::vec4 color;
	union {
		glm::vec4 position;
		glm::vec4 direction;
	};

	Light(const LightInfo& info, const LightType& type);
};

class PointLightShadow {
private:
	GLuint depthFBO;
	GLuint cubemap;
	float range;
	float refreshTime;

	void clearShadow(int lower, int upper, const LightInfo& info);
public:
	PointLightShadow(const LightInfo& info);
	bool update(float Frequency, std::array<int, 6>& updateValues, const LightInfo& info);
	void clean();
	void bind(const LightInfo& info) const;
	void unbind() const;
	GLuint getTexture() const;
	float getRadius() const;
};

#endif // !LIGHT_TYPES_HH
