#ifndef LIGHTS__H
#define LIGHTS__H

#include <vector>
#include <GL/glew.h>
#include "Includes/gCamera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

#include "Includes/VertexArrayObject.h"
#include "Includes/ProgramObject.h"
#include "LightTypes.h"
#include "Entity.h"
#include "LightBuffer.h"
#include "DirLightShadow.h"

enum LightType {
	POINT = 0,
	DIRECTIONAL = 1,
	POINT_SHADOW = 2,
	DIRECTIONAL_SHADOW = 3
};

class Lights {
	using Iterator = std::vector<LightInfo>::iterator;

private:
	bool frameBufferCreated{ false };
	GLuint lightFrameBuffer;
	GLuint lightTexture;

	std::array<ProgramObject, 4> shaders;
	ProgramObject pointShadow;
	ProgramObject dirShadow;

	std::array<LightBuffer, 4> buffers;

	std::vector<PointLightShadow> pointShadows;
	std::vector<DirLightShadow<5>> dirShadows;

	std::array<float, 4> shadowCascadeLevels;

	std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& projView);
	glm::mat4 getLightSpaceMatrix(const float nearPlane, const float farPlane, const glm::vec3& lightDir, const gCamera& camera);
	std::vector<glm::mat4> getLightSpaceMatrices(const glm::vec3& lightDir, const gCamera& camera);

	void renderPointLights(const GLuint diffuseBuffer, const GLuint normalBuffer, const GLuint depthBuffer, const gCamera& camera, LightType type);
	void renderDirectionalLights(const GLuint diffuseBuffer, const GLuint normalBuffer, const gCamera& camera, LightType type);
	void renderPointLightsShadowed(const GLuint diffuseBuffer, const GLuint normalBuffer, const GLuint depthBuffer, const gCamera& camera);
	void renderDirectionalLightsShadowed(const GLuint diffuseBuffer, const GLuint normalBuffer, const GLuint depthBuffer, const gCamera& camera);

public:
	Lights();
	~Lights();

	void renderLights(const GLuint diffuseBuffer, const GLuint normalBuffer, const GLuint depthBuffer, const gCamera& camera);
	void updateShadowMaps(const std::vector<Entity>& entities, const gCamera& camera);

	void createFrameBuffer(int width, int height, GLuint depthTexture);
	GLuint getLightTexture() const;
	std::vector<LightInfo>& getInfo(LightType type);

	void addLight(LightType type, const LightInfo& info = {});
	void updateLight(LightType type, int index);
	void deleteLight(LightType type, int index);

	void changeShadowed(LightType type, int index);
};
#endif // !LIGHTS__H
