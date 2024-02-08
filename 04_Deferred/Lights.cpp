#include "Lights.h"

Lights::Lights() {
	shaders[POINT].Init({
		{ GL_VERTEX_SHADER,		     "Shaders/deferred_point2.vert" },
		{ GL_TESS_CONTROL_SHADER,    "Shaders/deferred_point.tesc"},
		{ GL_TESS_EVALUATION_SHADER, "Shaders/deferred_point.tese"},
		{ GL_FRAGMENT_SHADER,	     "Shaders/deferred_point.frag" }
	});

	shaders[DIRECTIONAL].Init({
		{ GL_VERTEX_SHADER,   "Shaders/deferred_dir.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/deferred_dir.frag" }
	});

	shaders[POINT_SHADOW].Init({
		{ GL_VERTEX_SHADER,		     "Shaders/deferred_shadow_point.vert" },
		{ GL_TESS_CONTROL_SHADER,    "Shaders/deferred_shadow_point.tesc"},
		{ GL_TESS_EVALUATION_SHADER, "Shaders/deferred_shadow_point.tese"},
		{ GL_FRAGMENT_SHADER,	     "Shaders/deferred_shadow_point.frag" }
	});

	shaders[DIRECTIONAL_SHADOW].Init({
		{ GL_VERTEX_SHADER,   "Shaders/deferred_shadow_dir.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/deferred_shadow_dir.frag" }
	});

	pointShadow.Init({
		{ GL_VERTEX_SHADER,   "Shaders/shadow_point.vert"},
		{ GL_GEOMETRY_SHADER, "Shaders/shadow_point.geom"},
		{ GL_FRAGMENT_SHADER, "Shaders/empty.frag"}
	});

	dirShadow.Init({
		{ GL_VERTEX_SHADER,   "Shaders/shadow_point.vert"},
		{ GL_GEOMETRY_SHADER, "Shaders/shadow_dir.geom"},
		{ GL_FRAGMENT_SHADER, "Shaders/shadow_dir.frag"}
	});

	shadowCascadeLevels = { 1000.f / 50.0f, 1000.f / 25.0f, 1000.f / 10.0f, 1000.f / 2.0f };
}

Lights::~Lights() {
	if (frameBufferCreated) {
		glDeleteTextures(1, &lightTexture);
		glDeleteFramebuffers(1, &lightFrameBuffer);
	}
}

void Lights::createFrameBuffer(int width, int height, GLuint depthTexture) {
	if (frameBufferCreated) {
		glDeleteTextures(1, &lightTexture);
		glDeleteFramebuffers(1, &lightFrameBuffer);
	}

	glGenFramebuffers(1, &lightFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, lightFrameBuffer);

	glGenTextures(1, &lightTexture);
	glBindTexture(GL_TEXTURE_2D, lightTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightTexture, 0);
	if (glGetError() != GL_NO_ERROR)
	{
		std::cout << "Error creating color attachment" << std::endl;
		exit(1);
	}

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthTexture, 0);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "Error creating depth attachment" << std::endl;
		exit(1);
	}

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Incomplete framebuffer (";
		switch (status) {
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:			std::cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";		 break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:	std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
		case GL_FRAMEBUFFER_UNSUPPORTED:					std::cout << "GL_FRAMEBUFFER_UNSUPPORTED";					 break;
		}
		std::cout << ")" << std::endl;
		exit(1);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	frameBufferCreated = true;
}

void Lights::renderLights(const GLuint diffuseBuffer, const GLuint normalBuffer, const GLuint depthBuffer, const gCamera& camera) {
	glBindFramebuffer(GL_FRAMEBUFFER, lightFrameBuffer);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	renderPointLights(diffuseBuffer, normalBuffer, depthBuffer, camera, POINT);
	renderDirectionalLights(diffuseBuffer, normalBuffer, camera, DIRECTIONAL);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, 0, 0xFF);
	glStencilMask(0x00);

	renderPointLightsShadowed(diffuseBuffer, normalBuffer, depthBuffer, camera);
	renderDirectionalLightsShadowed(diffuseBuffer, normalBuffer, depthBuffer, camera);

	glStencilFunc(GL_EQUAL, 1, 0xFF);

	renderPointLights(diffuseBuffer, normalBuffer, depthBuffer, camera, POINT_SHADOW);
	renderDirectionalLights(diffuseBuffer, normalBuffer, camera, DIRECTIONAL_SHADOW);

	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
}

std::array<glm::mat4, 6> getTransform(const LightInfo& info, const PointLightShadow& shadow, const gCamera& camera) {
	constexpr float aspect = 1.f;
	constexpr float near = 0.01f;
	const float far = shadow.getRadius();

	std::array<glm::mat4, 6> transforms;
	glm::mat4 perspective = glm::perspective(glm::radians(90.0f), aspect, near, far);

	transforms[0] = perspective * glm::lookAt(info.position, info.position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
	transforms[1] = perspective * glm::lookAt(info.position, info.position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
	transforms[2] = perspective * glm::lookAt(info.position, info.position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
	transforms[3] = perspective * glm::lookAt(info.position, info.position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
	transforms[4] = perspective * glm::lookAt(info.position, info.position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
	transforms[5] = perspective * glm::lookAt(info.position, info.position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));

	return transforms;
}

void clearPointShadow(GLuint texture, GLuint start, GLsizei end, GLsizei width) {
	static float depthVal = 1.0f;
	glClearTexSubImage(texture, 0, 0, 0, start, width, width, end - start, GL_DEPTH_COMPONENT, GL_FLOAT, &depthVal);
}

void Lights::updateShadowMaps(const std::vector<Entity>& entities, const gCamera& camera) {
	int windowValues[4];
	glGetIntegerv(GL_VIEWPORT, windowValues);
	//Point Shadow
	pointShadow.Use();
	const std::vector<LightInfo>& infos = buffers[POINT_SHADOW].getInfos();
	for (int i = 0; i < infos.size(); ++i) {
		std::array<int, 6> update;
		if(!pointShadows[i].update(infos[i].refreshFrequency, update, infos[i])) continue;
		pointShadows[i].bind(infos[i]);

		std::array<glm::mat4, 6> transforms = getTransform(infos[i], pointShadows[i], camera);

		glUniform1iv(pointShadow.GetLocation("update"), 6, update.data());
		glUniformMatrix4fv(pointShadow.GetLocation("shadowMatrices"), 6, GL_FALSE, (float*)transforms.data());
		pointShadow.SetUniform("lightPos", infos[i].position);
		pointShadow.SetUniform("radius", pointShadows[i].getRadius());

		for (const auto& entity : entities) {
			if (entity.castShadow) {
				pointShadow.SetUniform("M", entity.getLocalModelMatrix());
				entity.mesh->draw();
			}
		}
	}
	pointShadow.Unuse();
	//DIRECTIONAL SHADOW
	dirShadow.Use();
	const std::vector<LightInfo>& dirInfos = buffers[DIRECTIONAL_SHADOW].getInfos();
	for (int i = 0; i < dirInfos.size(); ++i) {
		DirLightShadow<5>::INT_A update;

		std::vector<glm::mat4> _transforms = getLightSpaceMatrices(glm::normalize(dirInfos[i].direction), camera);
		DirLightShadow<5>::MAT4_A transforms;
		for (int i = 0; i < transforms.size(); ++i) {
			transforms[i] = _transforms[i];
		}
		if (!dirShadows[i].update(dirInfos[i].refreshFrequency, update, dirInfos[i],transforms)) continue;

		dirShadows[i].bind(dirInfos[i]);

		glUniform1iv(dirShadow.GetLocation("update"), 6, update.data());
		glUniformMatrix4fv(dirShadow.GetLocation("lightSpaceMatrices"), 5, GL_FALSE, (float*)dirShadows[i].getTransfroms().data());
		for (const auto& entity : entities) {
			if (entity.castShadow) {
				dirShadow.SetUniform("M", entity.getLocalModelMatrix());
				entity.mesh->draw();
			}
		}
	}
	dirShadow.Unuse();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(windowValues[0], windowValues[1], windowValues[2], windowValues[3]);
}

void Lights::renderPointLights(const GLuint diffuseBuffer, const GLuint normalBuffer, const GLuint depthBuffer, const gCamera& camera, LightType type) {
	assert(type == POINT || type == POINT_SHADOW);
	buffers[type].bind(0);
	glDepthFunc(GL_GREATER);
	glDepthMask(GL_FALSE);

	shaders[POINT].Use();
	shaders[POINT].SetTexture("diffuseTexture", 1, diffuseBuffer);
	shaders[POINT].SetTexture("normalTexture", 2, normalBuffer);
	shaders[POINT].SetTexture("depthTexture", 3, depthBuffer);
	shaders[POINT].SetUniform("PI", glm::inverse(camera.GetProj()));
	shaders[POINT].SetUniform("view", camera.GetView());
	shaders[POINT].SetUniform("proj", camera.GetProj());

	glPatchParameteri(GL_PATCH_VERTICES, 1);
	glDrawArraysInstanced(GL_PATCHES, 0, buffers[type].getSize(), 1);
	shaders[POINT].Unuse();

	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	buffers[POINT].unbind();
}

void Lights::renderDirectionalLights(const GLuint diffuseBuffer, const GLuint normalBuffer, const gCamera& camera, LightType type) {
	assert(type == DIRECTIONAL || type == DIRECTIONAL_SHADOW);
	buffers[type].bind(0);
	glDisable(GL_DEPTH_TEST);
	shaders[DIRECTIONAL].Use();
	shaders[DIRECTIONAL].SetTexture("diffuseTexture", 1, diffuseBuffer);
	shaders[DIRECTIONAL].SetTexture("normalTexture", 2, normalBuffer);
	shaders[DIRECTIONAL].SetUniform("VIT", glm::transpose(glm::inverse(camera.GetView())));

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, buffers[type].getSize());
	shaders[DIRECTIONAL].Unuse();
	glEnable(GL_DEPTH_TEST);
}

void Lights::renderPointLightsShadowed(const GLuint diffuseBuffer, const GLuint normalBuffer, const GLuint depthBuffer, const gCamera& camera) {
	glDepthFunc(GL_GREATER);
	glDepthMask(GL_FALSE);

	shaders[POINT_SHADOW].Use();
	shaders[POINT_SHADOW].SetTexture("diffuseTexture", 1, diffuseBuffer);
	shaders[POINT_SHADOW].SetTexture("normalTexture", 2, normalBuffer);
	shaders[POINT_SHADOW].SetTexture("depthTexture", 3, depthBuffer);
	shaders[POINT_SHADOW].SetUniform("PI", glm::inverse(camera.GetProj()));
	shaders[POINT_SHADOW].SetUniform("view", camera.GetView());
	shaders[POINT_SHADOW].SetUniform("proj", camera.GetProj());
	shaders[POINT_SHADOW].SetUniform("VI", glm::inverse(camera.GetView()));
	const std::vector<LightInfo>& infos = buffers[POINT_SHADOW].getInfos();
	for (int i = 0; i < infos.size(); ++i) {
		shaders[POINT_SHADOW].SetCubeTexture("depthMap", 0, pointShadows[i].getTexture());
		shaders[POINT_SHADOW].SetUniform("color_in", infos[i].color);
		shaders[POINT_SHADOW].SetUniform("position_in", infos[i].position);
		shaders[POINT_SHADOW].SetUniform("radius", pointShadows[i].getRadius());
		glDrawArrays(GL_PATCHES, 0, 1);
	}
	shaders[POINT_SHADOW].Unuse();
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
}

void Lights::renderDirectionalLightsShadowed(const GLuint diffuseBuffer, const GLuint normalBuffer, const GLuint depthBuffer, const gCamera& camera) {
	glDisable(GL_DEPTH_TEST);

	shaders[DIRECTIONAL_SHADOW].Use();
	shaders[DIRECTIONAL_SHADOW].SetTexture("diffuseTexture", 0, diffuseBuffer);
	shaders[DIRECTIONAL_SHADOW].SetTexture("normalTexture", 1, normalBuffer);
	shaders[DIRECTIONAL_SHADOW].SetTexture("depthTexture", 2, depthBuffer);
	shaders[DIRECTIONAL_SHADOW].SetUniform("V", camera.GetView());
	shaders[DIRECTIONAL_SHADOW].SetUniform("PI", glm::inverse(camera.GetProj()));
	shaders[DIRECTIONAL_SHADOW].SetUniform("VI", glm::inverse(camera.GetView()));
	shaders[DIRECTIONAL_SHADOW].SetUniform("VIT", glm::transpose(glm::inverse(camera.GetView())));
	const std::vector<LightInfo>& infos = buffers[DIRECTIONAL_SHADOW].getInfos();
	for (int i = 0; i < infos.size(); ++i) {
		std::vector<glm::mat4> transforms = getLightSpaceMatrices(glm::normalize(infos[i].direction), camera);
		glUniformMatrix4fv(shaders[DIRECTIONAL_SHADOW].GetLocation("lightSpaceMatrices"), 5, GL_FALSE, (float*)transforms.data());

		shaders[DIRECTIONAL_SHADOW].setTextureArray("shadowTexture", 3, dirShadows[i].getTexture());
		shaders[DIRECTIONAL_SHADOW].SetUniform("color", infos[i].color);
		glm::vec3 dir = glm::normalize(infos[i].direction);
		shaders[DIRECTIONAL_SHADOW].SetUniform("direction", dir);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	shaders[DIRECTIONAL_SHADOW].Unuse();

	glEnable(GL_DEPTH_TEST);
}

GLuint Lights::getLightTexture() const {
	return lightTexture;
}

std::vector<LightInfo>& Lights::getInfo(LightType type) {
	//return lightInfos[type];
	return buffers[type].getInfos();
}

void Lights::addLight(LightType type, const LightInfo& info) {
	buffers[type].addLight(type, info);
	switch (type)
	{
	case POINT_SHADOW:
		pointShadows.push_back(info);
		break;
	case DIRECTIONAL_SHADOW:
		dirShadows.push_back(info);
		break;
	default:
		break;
	}
}

void Lights::updateLight(LightType type, int index) {
	buffers[type].updateLight(type, index);
	switch (type)
	{
	case POINT_SHADOW:
		pointShadows[index].clean();
		pointShadows[index] = buffers[POINT_SHADOW].getInfos()[index];
		break;
	case DIRECTIONAL_SHADOW:
		dirShadows[index].clean();
		dirShadows[index] = buffers[DIRECTIONAL_SHADOW].getInfos()[index];
		break;
	default:
		break;
	}
}

void Lights::deleteLight(LightType type, int index) {
	buffers[type].deleteLight(type, index);
	switch (type)
	{
	case POINT_SHADOW:
		pointShadows.erase(pointShadows.begin() + index);
		break;
	case DIRECTIONAL_SHADOW:
		dirShadows.erase(dirShadows.begin() + index);
		break;
	default:
		break;
	}
}

void Lights::changeShadowed(LightType type, int index) {
	LightInfo current = buffers[type].getInfos()[index];
	deleteLight(type, index);
	switch (type)
	{
	case POINT:
		addLight(POINT_SHADOW, current);
		break;
	case DIRECTIONAL:
		addLight(DIRECTIONAL_SHADOW, current);
		break;
	case POINT_SHADOW:
		addLight(POINT, current);
		break;
	case DIRECTIONAL_SHADOW:
		addLight(DIRECTIONAL, current);
	}
}

std::vector<glm::vec4> Lights::getFrustumCornersWorldSpace(const glm::mat4& projView) {
	const auto inv = glm::inverse(projView);

	std::vector<glm::vec4> frustumCorners;
	for (unsigned int x = 0; x < 2; ++x)
	{
		for (unsigned int y = 0; y < 2; ++y)
		{
			for (unsigned int z = 0; z < 2; ++z)
			{
				const glm::vec4 pt =
					inv * glm::vec4(
						2.0f * x - 1.0f,
						2.0f * y - 1.0f,
						2.0f * z - 1.0f,
						1.0f);
				frustumCorners.push_back(pt / pt.w);
			}
		}
	}

	return frustumCorners;
}

glm::mat4 Lights::getLightSpaceMatrix(const float nearPlane, const float farPlane, const glm::vec3& lightDir, const gCamera& camera) {
	const std::vector<glm::vec4> corners = getFrustumCornersWorldSpace(camera.GetFrustumRange(nearPlane, farPlane));

	glm::vec3 center = glm::vec3(0, 0, 0);
	for (const auto& v : corners)
	{
		center += glm::vec3(v);
	}
	center /= corners.size();

	const auto lightView = glm::lookAt(center + lightDir, center, glm::vec3(0.0f, 1.0f, 0.0f));

	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::lowest();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::lowest();
	for (const auto& v : corners)
	{
		const glm::vec4 trf = lightView * v;
		minX = std::min(minX, trf.x);
		maxX = std::max(maxX, trf.x);
		minY = std::min(minY, trf.y);
		maxY = std::max(maxY, trf.y);
		minZ = std::min(minZ, trf.z);
		maxZ = std::max(maxZ, trf.z);
	}

	// Tune this parameter according to the scene
	constexpr float zMult = 10.0f;
	if (minZ < 0) {
		minZ *= zMult;
	} else {
		minZ /= zMult;
	}
	if (maxZ < 0) {
		maxZ /= zMult;
	} else {
		maxZ *= zMult;
	}

	const glm::mat4 lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
	return lightProjection * lightView;
}

std::vector<glm::mat4> Lights::getLightSpaceMatrices(const glm::vec3& lightDir, const gCamera& camera) {
	std::vector<glm::mat4> ret;
	for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
	{
		if (i == 0)
		{
			ret.push_back(getLightSpaceMatrix(camera.getNearPlane(), shadowCascadeLevels[i], lightDir, camera));
		}
		else if (i < shadowCascadeLevels.size())
		{
			ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[i], lightDir, camera));
		}
		else
		{
			ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], camera.getFarPlane(), lightDir, camera));
		}
	}
	return ret;
}