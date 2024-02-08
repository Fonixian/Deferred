#include "LightBuffer.h"

LightBuffer::LightBuffer() {
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	lastSize = 0;
}

LightBuffer::~LightBuffer() {
	glDeleteBuffers(1, &buffer);
}

void LightBuffer::bind(GLuint base) {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, base, buffer);
}

void LightBuffer::unbind() {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void LightBuffer::addLight(const LightType& type, const LightInfo& info) {
	++lastSize;
	infos.emplace_back(info);

	std::vector<Light> lights;
	lights.reserve(lastSize);
	for (const auto& info : infos) {
		lights.emplace_back(info, type);
	}
	glNamedBufferData(buffer, lights.size() * sizeof(Light), lights.data(), GL_STATIC_DRAW);
}

void LightBuffer::updateLight(const LightType& type, int index) {
	Light light{ infos[index], type };
	glNamedBufferSubData(buffer, index * sizeof(Light), sizeof(Light), &light);
}

void LightBuffer::deleteLight(const LightType& type, int index) {
	--lastSize;
	infos.erase(infos.begin() + index);

	std::vector<Light> lights;
	lights.reserve(lastSize);
	for (const auto& light : infos) {
		lights.emplace_back(light, type);
	}
	glNamedBufferSubData(buffer, 0, lastSize * sizeof(Light), lights.data());
}

std::vector<LightInfo>& LightBuffer::getInfos() {
	return infos;
}

int LightBuffer::getSize() {
	return lastSize;
}