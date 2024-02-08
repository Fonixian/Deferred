#include "LightTypes.h"
#include "Lights.h"

LightInfo::LightInfo() {
	color    = glm::vec3(0, 0, 0);
	position = glm::vec3(0, 0, 0);
	cast_shadow = false;
	shadow_map_resolution_wh[0] = 1024;
	shadow_map_resolution_wh[1] = 1024;
	refreshFrequency = 1.f;
}

LightInfo::LightInfo(glm::vec3 color, glm::vec3 pos_dir, bool cast_shadow, std::vector<int> resolution_wh) : color(color), position(pos_dir), cast_shadow(cast_shadow) {
	shadow_map_resolution_wh[0] = resolution_wh[0];
	shadow_map_resolution_wh[1] = resolution_wh[1];
	refreshFrequency = 1.f;
}

Light::Light(const LightInfo& info, const LightType& type) {
	float radius = info.color.r + info.color.g + info.color.b;
	radius = sqrt(radius * 25);

	color = glm::vec4(info.color, radius);

	switch (type)
	{
	case POINT:
	case POINT_SHADOW:
		position = glm::vec4(info.position, 1);
		break;
	case DIRECTIONAL:
	case DIRECTIONAL_SHADOW:
		direction = glm::vec4(info.position, 0);
		break;
	}
}

PointLightShadow::PointLightShadow(const LightInfo& info) {
	range = sqrt((info.color.r + info.color.g + info.color.b) * 25);
	refreshTime = 0.f;

	glGenTextures(1, &cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT24,
			info.shadow_map_resolution_wh[0], info.shadow_map_resolution_wh[0], 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glCreateFramebuffers(1, &depthFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubemap, 0);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "Error creating depth attachment" << std::endl;
		exit(1);
	}
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);


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
}

bool PointLightShadow::update(float Frequency, std::array<int, 6>& updateValues, const LightInfo& info) {
	bool update = (int)(refreshTime + 6.0f / Frequency) > (int)refreshTime;
	int lower = (int)refreshTime;
	refreshTime += 6.0f / Frequency;
	int upper = (int)refreshTime % 6;

	refreshTime = std::fmod(refreshTime, 6.0f);
	if (!update) return update;

	updateValues.fill(0);
	if (lower < upper) {
		for (int i = lower; i < upper; ++i)updateValues[i] = 1;
		clearShadow(lower, upper, info);
	} else if (lower == upper) {
		updateValues.fill(1);
		clearShadow(0, 6, info);
	} else {
		for (int i = lower; i < 6; ++i)updateValues[i] = 1;
		for (int i = 0; i < upper; ++i)updateValues[i] = 1;
		clearShadow(lower, 6, info);
		clearShadow(0, upper, info);
	}

	return update;
}

void PointLightShadow::clearShadow(int lower, int upper, const LightInfo& info) { //[lower, upper[
	if (upper == 0) return;
	static float clearValue = 1.0f;
	glClearTexSubImage(cubemap, 0, 0, 0, lower, info.shadow_map_resolution_wh[0], info.shadow_map_resolution_wh[0],
		upper - lower, GL_DEPTH_COMPONENT, GL_FLOAT, &clearValue);
}

void PointLightShadow::clean() {
	glDeleteTextures(1, &cubemap);
	glDeleteFramebuffers(1, &depthFBO);
}

void PointLightShadow::bind(const LightInfo& info) const {
	glViewport(0, 0, info.shadow_map_resolution_wh[0], info.shadow_map_resolution_wh[0]);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
}

void PointLightShadow::unbind() const {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint PointLightShadow::getTexture() const{
	return cubemap;
}

float PointLightShadow::getRadius() const {
	return range;
}