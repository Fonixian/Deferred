#include "Mesh_OGL3.h"

Mesh::Mesh(void)
{
}

Mesh::~Mesh(void)
{
	if (inited)
	{
		glDeleteVertexArrays(1, &vertexArrayObject);

		glDeleteBuffers(1, &vertexBuffer);
		glDeleteBuffers(1, &indexBuffer);
	}
}

void Mesh::initBuffers()
{
	glGenVertexArrays(1, &vertexArrayObject);
	glGenBuffers(1, &vertexBuffer);
	glGenBuffers(1, &indexBuffer);

	glBindVertexArray(vertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*vertices.size(), (void*)&vertices[0], GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices.size(), (void*)&indices[0], GL_STREAM_DRAW);

	glBindVertexArray(0);

	inited = true;

	float minX = std::numeric_limits<float>::max();
	float maxX = std::numeric_limits<float>::lowest();
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::lowest();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = std::numeric_limits<float>::lowest();

	for (const auto& vertex : vertices) {
		minX = std::min(minX, vertex.position.x);
		maxX = std::max(maxX, vertex.position.x);
		minY = std::min(minY, vertex.position.y);
		maxY = std::max(maxY, vertex.position.y);
		minZ = std::min(minZ, vertex.position.z);
		maxZ = std::max(maxZ, vertex.position.z);
	}

	_center = { (minX + maxX) / 2.f, (minY + maxY) / 2.f, (minZ + maxZ) / 2.f };
	_radius = glm::length(glm::vec3{ maxX - minX, maxY - minY, maxZ - minZ }) / 2.0f;
}

void Mesh::draw()
{
	glBindVertexArray(vertexArrayObject);

	glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void Mesh::drawInstanced(int count) {
	glBindVertexArray(vertexArrayObject);

	glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0, count);

	glBindVertexArray(0);
}

void Mesh::getBoundingSphere(glm::vec3& center, float& radius) const{
	center = _center;
	radius = _radius;
}