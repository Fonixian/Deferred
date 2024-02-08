#ifndef DIRECTIONAL_LIGHT_SHADOW__H
#define DIRECTIONAL_LIGHT_SHADOW__H

#include <GL/glew.h>
#include "Includes/gCamera.h"
#include "LightTypes.h"
#include "Includes/ProgramObject.h"

template<int size>
class DirLightShadow {
public:
	using MAT4_A = std::array<glm::mat4, size>;
	using INT_A = std::array<int, size>;

	DirLightShadow(const LightInfo& info) {
		refreshTime = 0.f;

		createTextures(info);
	}

	bool update(float Frequency, INT_A& updateValues, const LightInfo& info, const MAT4_A& transforms) {
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
		}
		else if (lower == upper) {
			updateValues.fill(1);
			clearShadow(0, size, info);
		}
		else {
			for (int i = lower; i < size; ++i)updateValues[i] = 1;
			for (int i = 0; i < upper; ++i)updateValues[i] = 1;
			clearShadow(lower, size, info);
			clearShadow(0, upper, info);
		}

		updateTransforms(updateValues, transforms);
		return update;
	}

	void clean() {
		glDeleteTextures(1, &depthTextures);
		glDeleteFramebuffers(1, &depthFBO);
	}

	void bind(const LightInfo& info) const {
		glViewport(0, 0, info.shadow_map_resolution_wh[0], info.shadow_map_resolution_wh[1]);
		glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	}

	void unbind() const {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	GLuint getTexture() const {
		return depthTextures;
	}

	MAT4_A& getTransfroms() {
		return _transforms;
	}

private:
	GLuint depthFBO;
	GLuint depthTextures;
	MAT4_A _transforms;
	float refreshTime;

	void clearShadow(int lower, int upper, const LightInfo& info) { //[lower, upper[
		if (upper == 0) return;
		static float clearValue = 1.0f;
		glClearTexSubImage(depthTextures, 0, 0, 0, lower, info.shadow_map_resolution_wh[0], info.shadow_map_resolution_wh[1],
			upper - lower, GL_DEPTH_COMPONENT, GL_FLOAT, &clearValue);
	}

	void createTextures(const LightInfo& info) {
		glGenFramebuffers(1, &depthFBO);

		glGenTextures(1, &depthTextures);
		glBindTexture(GL_TEXTURE_2D_ARRAY, depthTextures);

		glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32, info.shadow_map_resolution_wh[0], info.shadow_map_resolution_wh[1], size);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, bordercolor);

		glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTextures, 0);
		if (glGetError() != GL_NO_ERROR) {
			std::cout << "Error creating depth attachment" << std::endl;
			exit(1);
		}
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
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

	void updateTransforms(const INT_A& updates, const MAT4_A& transforms) {
		for (int i = 0; i < size; ++i) {
			if (updates[i] == 1)_transforms[i] = transforms[i];
		}
	}
};

#endif // DIRECTIONAL_LIGHT_SHADOW__H
