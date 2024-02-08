#include "MyApp.h"

#include <math.h>
#include <vector>

#include <array>
#include <list>
#include <tuple>

#include "imgui/imgui.h"
#include "Includes/ObjParser_OGL3.h"
#include <string>
#include <random>
CMyApp::CMyApp(void){}

CMyApp::~CMyApp(void){}

bool CMyApp::Init()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.);
	m_program.Init({
		{ GL_VERTEX_SHADER,   "Shaders/myVert.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/myFrag.frag" }
	});

	nonReflective.Init({
		{ GL_VERTEX_SHADER,   "Shaders/myVert.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/myFrag.frag" }
	});

	reflective.Init({
		{ GL_VERTEX_SHADER,   "Shaders/reflective.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/reflective.frag" }
	});

	m_programPostprocess.Init({
		{ GL_VERTEX_SHADER,   "Shaders/postprocess.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/postprocess.frag" }
	});

	constexpr int N = 100;
	constexpr int M = 100;
	std::array<Mesh::Vertex, (N + 1)* (M + 1)> vert;
	for (int j = 0; j <= M; ++j)
	{
		for (int i = 0; i <= N; ++i)
		{
			float x = i / (float)N * 300.f - 200.f;
			float z = j / (float)M * 300.f - 200.f;
			int index = i + j * (N + 1);
			glm::vec4 nh = getNormalHeight(x, z);
			vert[index] = { glm::vec3(x,nh.w,z) , glm::vec3(nh.x,nh.y,nh.z), glm::vec2(i / (float)N,j / (float)M) };
		}
	}

	// indexpuffer adatai: NxM négyszög = 2xNxM háromszög = háromszöglista esetén 3x2xNxM index
	std::array<unsigned int, 3 * 2 * (N) * (M)> indices;
	for (int j = 0; j < M; ++j)
	{
		for (int i = 0; i < N; ++i)
		{
			int index = i * 6 + j * (6 * N);
			indices[index + 2] = (i)+(j) * (N + 1);
			indices[index + 1] = (i + 1) + (j) * (N + 1);
			indices[index + 0] = (i)+(j + 1) * (N + 1);
			indices[index + 5] = (i + 1) + (j) * (N + 1);
			indices[index + 4] = (i + 1) + (j + 1) * (N + 1);
			indices[index + 3] = (i)+(j + 1) * (N + 1);
		}
	}
	std::shared_ptr<Mesh> landscape = std::make_shared<Mesh>();
	for (const Mesh::Vertex& v : vert) {
		landscape->addVertex(v);
	}
	for (const auto& ind : indices) {
		landscape->addIndex(ind);
	}
	landscape->initBuffers();
	
	//Load Textures
	std::shared_ptr<Texture2D> metalTexture(std::make_shared<Texture2D>("Assets/texture.png"));
	std::shared_ptr<Texture2D> grassTexture(std::make_shared<Texture2D>("Assets/grass.png"));
	std::shared_ptr<Texture2D> treeTexture(std::make_shared<Texture2D>("Assets/tree.bmp"));

	//Load Meshes
	std::shared_ptr<Mesh> suzanne{std::move(ObjParser::parse("Assets/Suzanne.obj"))};
	std::shared_ptr<Mesh> sphere{std::move(ObjParser::parse("Assets/sphere.obj"))};
	std::shared_ptr<Mesh> cube{std::move(ObjParser::parse("Assets/cube.obj"))};
	std::shared_ptr<Mesh> treeA{std::move(ObjParser::parse("Assets/tree1.obj"))};
	std::shared_ptr<Mesh> treeB{std::move(ObjParser::parse("Assets/tree2.obj"))};

	//Loading Entities
	for (int i = -1; i <= 1; ++i)
		for (int j = -1; j <= 1; ++j)
		{
			if ((i + j) % 2 == 0) {
				entities.emplace_back(suzanne,metalTexture,glm::vec3( 50 + 4 * i, 4 * (j + 1) + getHeight(50, -80) + 5.f, -80), glm::vec3(0.f), glm::vec3(1.f));
			} else {
				entities.emplace_back(sphere, grassTexture, glm::vec3(50 + 4 * i, 4 * (j + 1) + getHeight(50, -80) + 5.f, -80), glm::vec3(0.f), glm::vec3(1.f));
			}
		}

	const float x[5] = { -180.f, -80.f, -150.f, -30.f, -164.f };
	const float z[5] = { -180.f, -30.f, -90.f, -100.f, -54.f };
	for (int i = 0; i < 5; ++i) {
		if (i % 2 == 0) {
			entities.emplace_back(treeA, treeTexture, glm::vec3(x[i], getHeight(x[i], z[i]), z[i]) - 0.2f, glm::vec3(0, x[i], 0), glm::vec3(1));
		} else {
			entities.emplace_back(treeB, treeTexture, glm::vec3(x[i], getHeight(x[i], z[i]), z[i]) - 0.2f, glm::vec3(0, z[i], 0), glm::vec3(1));
		}
	}

	entities.emplace_back(cube, metalTexture, glm::vec3(50, getHeight(50, -80) + 1.f, -80), glm::vec3(0.f), glm::vec3(20,2,20));

	entities.emplace_back(landscape, grassTexture, glm::vec3(0.f), glm::vec3(0.f), glm::vec3(1.f)).castShadow = false;

	entities.emplace_back(cube, metalTexture, glm::vec3(0, getHeight(0,10.0f) + 6.f, 0), glm::vec3(0), glm::vec3(3));
	entities.emplace_back(cube, metalTexture, glm::vec3(0, getHeight(0,12.5f) + 6.f, 12.5f), glm::vec3(0), glm::vec3(1));

	entities[entities.size() - 1].GenerateReflection(true);
	entities[entities.size() - 2].GenerateReflection(true);

	// Loading Textures

	int windowValues[4];
	glGetIntegerv(GL_VIEWPORT, windowValues);
	Resize(windowValues[2], windowValues[3]);

	lights.addLight(DIRECTIONAL_SHADOW, LightInfo(glm::vec3(0.6f),glm::vec3(0,1,1),true,{1024,1024}));

	return true;
}

void CMyApp::Clean()
{
	if (frameBufferCreated) {
		glDeleteTextures(1, &diffuseTexture);
		glDeleteTextures(1, &normalTextureView);
		glDeleteTextures(1, &depthTexturePers);
		glDeleteFramebuffers(1, &sceneFrameBuffer);
	}
}

void CMyApp::Update()
{
	static Uint32 last_time = SDL_GetTicks();
	float delta_time = (SDL_GetTicks() - last_time) / 1000.0f;

	m_camera.Update(delta_time);
	
	last_time = SDL_GetTicks();

	int c = 0;
	float t = last_time / 1000.f;
	for (int i = -1; i <= 1; ++i)
		for (int j = -1; j <= 1; ++j)
		{
			entities[c].pos.z = sinf(t * 2 * M_PI * i * j) - 80.f;
			++c;
		}
}

void CMyApp::DrawScene(const glm::mat4& view, const glm::mat4& proj, const bool receiveShadow) {
	glm::mat4 viewProj = proj * view;
	glm::mat4 world;

	if (!receiveShadow) {
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
	}

	nonReflective.Use();
	for (const auto& entity : entities) {
		if (entity.receiveShadow == receiveShadow && !entity.getGenerateReflection()) {
			entity.setTexture(nonReflective);
			world = entity.getLocalModelMatrix();
			nonReflective.SetUniform("MVP", viewProj * world);
			nonReflective.SetUniform("worldIT", glm::transpose(glm::inverse(world)));
			nonReflective.SetUniform("MVIT", glm::transpose(glm::inverse(view * world)));
			entity.mesh->draw();
		}
	}
	nonReflective.Unuse();

	reflective.Use();
	for (const auto& entity : entities) {
		if (entity.receiveShadow == receiveShadow && entity.getGenerateReflection()) {
			entity.setTexture(reflective);
			world = entity.getLocalModelMatrix();
			glm::mat4 mat = m_camera.GetView() * world;
			reflective.SetUniform("MV", mat);
			reflective.SetUniform("VI", glm::inverse(m_camera.GetView()));
			reflective.SetUniform("MVP", viewProj * world);
			reflective.SetUniform("worldIT", glm::transpose(glm::inverse(world)));
			mat = glm::transpose(glm::inverse(mat));
			reflective.SetUniform("MVIT", mat);
			reflective.SetCubeTexture("environmentMap", 1, entity.environmentMap->getTexture());
			entity.mesh->draw();
		}
	}
	reflective.Unuse();

	if (!receiveShadow) {
		glDisable(GL_STENCIL_TEST);
	}
	Entity::Reset();
}

void CMyApp::renderLightGUI(LightType type){
	bool shadowChange = false;
	int index;

	std::vector<LightInfo>& currentLights = lights.getInfo(type);
	for (int i = 0; i < currentLights.size(); ++i) {
		ImGui::PushID(&currentLights[i]);

		if (ImGui::InputFloat3("R G B", (float*)&currentLights[i].color)) {
			lights.updateLight(type, i);
		}
		if (ImGui::InputFloat3("X Y Z", (float*)&currentLights[i].position)) {
			lights.updateLight(type, i);
		}
		if (ImGui::Checkbox("Shadow", &currentLights[i].cast_shadow)) {
			shadowChange = true;
			index = i;
		}
		switch (type)
		{
		case POINT:
		case POINT_SHADOW:
			if (ImGui::InputInt("Shadow resolution", currentLights[i].shadow_map_resolution_wh)) {
				lights.updateLight(type, i);
			}
			break;
		case DIRECTIONAL:
		case DIRECTIONAL_SHADOW:
			if (ImGui::InputInt2("Shadow resolution", currentLights[i].shadow_map_resolution_wh)) {
				lights.updateLight(type, i);
			}
			break;
		}
		ImGui::DragFloat("Refresh ", &currentLights[i].refreshFrequency, .25f, 1.0f, 10.f);
		if (ImGui::Button("Delete")) {
			lights.deleteLight(type, i);
		}

		ImGui::PopID();
		ImGui::Separator();
	}

	if(shadowChange)lights.changeShadowed(type, index);
}

void CMyApp::renderEntityGUI() {
	for (int i = 0; i < entities.size(); ++i) {
		ImGui::PushID(&entities[i]);

		if (ImGui::InputFloat3("Pos: X Y Z", (float*)&entities[i].pos)) {
			entities[i].moved();
		}
		ImGui::InputFloat3("Rot: X Y Z", (float*)&entities[i].eulerRot);
		ImGui::InputFloat3("Scale: X Y Z", (float*)&entities[i].scale);
		ImGui::Checkbox("Cast Shadow", &entities[i].castShadow);
		ImGui::Checkbox("Receive Shadow", &entities[i].receiveShadow);
		ImGui::Checkbox("Refleced", &entities[i].reflected);
		if(!entities[i].getGenerateReflection()) {
			if (ImGui::Button("Reflection")) {
				entities[i].GenerateReflection(true);
			}
		} else {
			EnvironmentMap& envMap = *entities[i].environmentMap;
			if (ImGui::InputInt("Resolution", &envMap.resolution)) {
				envMap.createFrameBuffer(envMap.resolution);
			}
			ImGui::InputFloat("Frequency", &envMap.frequency);
			if (ImGui::Button("Reflection")) {
				entities[i].GenerateReflection(false);
			}
		}

		ImGui::PopID();
		ImGui::Separator();
	}
}

void CMyApp::Render()
{
	// Render to the framebuffer
	int windowValues[4];
	glGetIntegerv(GL_VIEWPORT, windowValues);
	for (auto& entity : entities) {
		entity.update(entities);
	}
	glViewport(windowValues[0], windowValues[1], windowValues[2], windowValues[3]);

	// Calc Lights
	lights.updateShadowMaps(entities, m_camera);
	
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFrameBuffer);
	glStencilMask(0xFF);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	DrawScene(m_camera.GetView(), m_camera.GetProj(), true);
	DrawScene(m_camera.GetView(), m_camera.GetProj(), false);

	lights.renderLights(diffuseTexture, normalTextureView, depthTexturePers, m_camera);
	// Calc SSAO
	ssao.renderSSAO(normalTextureView, depthTexturePers, m_camera);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Draw
	m_programPostprocess.Use();
	m_programPostprocess.SetTexture("frameTex", 0, lights.getLightTexture());
	m_programPostprocess.SetTexture("ssao", 1, ssao.getSSAO());
	m_programPostprocess.SetTexture("diffuse", 2, diffuseTexture);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_programPostprocess.Unuse();
	// UI
	ImGui::ShowTestWindow(); // Demo of all ImGui commands. See its implementation for details.
		// It's worth browsing imgui.h, as well as reading the FAQ at the beginning of imgui.cpp.
		// There is no regular documentation, but the things mentioned above should be sufficient.

	ImGui::SetNextWindowPos(ImVec2(300, 400), ImGuiSetCond_FirstUseEver);

	if(ImGui::Begin("Test window"))
	{
		glm::vec3 curent_position = m_camera.GetEye();
		ImGui::Text("X:%f Y:%f Z:%f", curent_position.x, curent_position.y, curent_position.z);
		if (ImGui::CollapsingHeader("Textures")) {
			ImGui::Image((ImTextureID)diffuseTexture , ImVec2(256, 256), ImVec2(0,1), ImVec2(1,0));
			ImGui::Image((ImTextureID)normalTextureView  , ImVec2(256, 256), ImVec2(0,1), ImVec2(1,0));
			ImGui::Image((ImTextureID)depthTexturePers   , ImVec2(256, 256), ImVec2(0,1), ImVec2(1,0));
		}

		if (ImGui::CollapsingHeader("Objects")) {
			renderEntityGUI();
		}

		if (ImGui::CollapsingHeader("Lights"))
		{
			if (ImGui::CollapsingHeader("Point Light")) {
				if (ImGui::Button("New point light")) {
					lights.addLight(POINT, { {10,10,10}, m_camera.GetEye(), false, {1024,1024}});
				}
				renderLightGUI(POINT);
				renderLightGUI(POINT_SHADOW);
			}

			if (ImGui::CollapsingHeader("Directional Light")) {
				if (ImGui::Button("New directional light")) {
					lights.addLight(DIRECTIONAL);
				}
				renderLightGUI(DIRECTIONAL);
				renderLightGUI(DIRECTIONAL_SHADOW);
			}
		}
	}
	ImGui::End(); // In either case, ImGui::End() needs to be called for ImGui::Begin().
		// Note that other commands may work differently and may not need an End* if Begin* returned false.
}

void CMyApp::KeyboardDown(SDL_KeyboardEvent& key)
{
	m_camera.KeyboardDown(key);
}

void CMyApp::KeyboardUp(SDL_KeyboardEvent& key)
{
	m_camera.KeyboardUp(key);
}

void CMyApp::MouseMove(SDL_MouseMotionEvent& mouse)
{
	m_camera.MouseMove(mouse);
}

void CMyApp::MouseDown(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseUp(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseWheel(SDL_MouseWheelEvent& wheel)
{
}

// _w and _h are the width and height of the window's size
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h );

	m_camera.Resize(_w, _h);
	
	CreateFrameBuffer(_w, _h);
}

inline void setTexture2DParameters(GLenum magfilter = GL_LINEAR, GLenum minfilter = GL_LINEAR, GLenum wrap_s = GL_CLAMP_TO_EDGE, GLenum wrap_t = GL_CLAMP_TO_EDGE)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
}

void CMyApp::CreateFrameBuffer(int width, int height)
{
	if ( frameBufferCreated ) {
		glDeleteTextures(1, &diffuseTexture);
		glDeleteTextures(1, &normalTextureView);
		glDeleteTextures(1, &depthTexturePers);
		glDeleteFramebuffers(1, &sceneFrameBuffer);
	}

	glGenFramebuffers(1, &sceneFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFrameBuffer);

	// Diffuse colors
	glGenTextures(1, &diffuseTexture);
	glBindTexture(GL_TEXTURE_2D, diffuseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	setTexture2DParameters(GL_NEAREST, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 0, GL_TEXTURE_2D, diffuseTexture, 0);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "Error creating color attachment 0" << std::endl;
		exit(1);
	}

	// Normal vectors
	glGenTextures(1, &normalTextureView);
	glBindTexture(GL_TEXTURE_2D, normalTextureView);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	setTexture2DParameters(GL_NEAREST, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1 , GL_TEXTURE_2D, normalTextureView, 0);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "Error creating color attachment 1" << std::endl;
		exit(1);
	}

	// Depth values
	glGenTextures(1, &depthTexturePers);
	glBindTexture(GL_TEXTURE_2D, depthTexturePers);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
	setTexture2DParameters(GL_NEAREST, GL_NEAREST);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, depthTexturePers, 0);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "Error creating depth attachment" << std::endl;
		exit(1);
	}

	GLenum drawBuffers[2] = { GL_COLOR_ATTACHMENT0,
							 GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, drawBuffers);

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
	lights.createFrameBuffer(width, height, depthTexturePers);
	ssao.createFrameBuffer(width, height);
}

float CMyApp::getHeight(float x, float z) {
	float u = (x + 200.f) / 300.f;
	float v = (z + 200.f) / 300.f;

	float Bu[4] = { (1 - u) * (1 - u) * (1 - u),3 * u * (1 - u) * (1 - u),3 * u * u * (1 - u),u * u * u };
	float Bv[4] = { (1 - v) * (1 - v) * (1 - v),3 * v * (1 - v) * (1 - v),3 * v * v * (1 - v),v * v * v };

	float dBu[3] = { (1 - u) * (1 - u), 2 * u * (1 - u),u * u };
	float dBv[3] = { (1 - v) * (1 - v), 2 * v * (1 - v),v * v };

	glm::vec3 dub[3 * 4];
	glm::vec3 dvb[4 * 3];

	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 4; ++j)
			dub[i + j * 3] = 3.0f * (landscapePoints[i + 1 + j * 4] - landscapePoints[i + j * 4]);

	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 3; ++j)
			dvb[i + j * 4] = 3.0f * (landscapePoints[i + (j + 1) * 4] - landscapePoints[i + j * 4]);

	glm::vec3 position{0, 0, 0};

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			position += landscapePoints[i + j * 4] * Bu[i] * Bv[j];
		}
	}
	return position.y;
}

glm::vec4 CMyApp::getNormalHeight(float x, float z) {
	float u = (x + 200.f) / 300.f;
	float v = (z + 200.f) / 300.f;

	float Bu[4] = { (1 - u) * (1 - u) * (1 - u),3 * u * (1 - u) * (1 - u),3 * u * u * (1 - u),u * u * u };
	float Bv[4] = { (1 - v) * (1 - v) * (1 - v),3 * v * (1 - v) * (1 - v),3 * v * v * (1 - v),v * v * v };

	float dBu[3] = { (1 - u) * (1 - u), 2 * u * (1 - u),u * u };
	float dBv[3] = { (1 - v) * (1 - v), 2 * v * (1 - v),v * v };

	glm::vec3 dub[3 * 4];
	glm::vec3 dvb[4 * 3];

	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 4; ++j)
			dub[i + j * 3] = 3.0f * (landscapePoints[i + 1 + j * 4] - landscapePoints[i + j * 4]);

	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 3; ++j)
			dvb[i + j * 4] = 3.0f * (landscapePoints[i + (j + 1) * 4] - landscapePoints[i + j * 4]);

	glm::vec3 position{0, 0, 0};

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			position += landscapePoints[i + j * 4] * Bu[i] * Bv[j];
		}
	}

	glm::vec3 du(0, 0, 0);
	glm::vec3 dv(0,0,0);

	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 4; ++j)
			du += dub[i + j * 3] * dBu[i] * Bv[j];

	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 3; ++j)
			dv += dvb[i + j * 4] * Bu[i] * dBv[j];
	glm::vec3 normal = glm::normalize(cross(du, dv));

	return { normal, position.y };
}