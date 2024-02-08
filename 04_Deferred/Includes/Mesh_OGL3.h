#pragma once

#include <GL/glew.h>

#include <vector>
#include <glm/glm.hpp>

class Mesh
{
public:
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texcoord;
	};

	Mesh(void);
	~Mesh(void);

	void initBuffers();
	void draw();
	void drawInstanced(int count);
	void getBoundingSphere(glm::vec3& center, float& radius) const;

	void addVertex(const Vertex& vertex) {
		vertices.push_back(vertex);
	}
	void addIndex(unsigned int index) {
		indices.push_back(index);
	}
private:
	GLuint vertexArrayObject;
	GLuint vertexBuffer;
	GLuint indexBuffer;

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	bool inited = false;

	glm::vec3 _center = glm::vec3(0,0,0);
	float _radius = -1.0f;
};
