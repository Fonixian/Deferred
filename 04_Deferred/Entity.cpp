#include "Entity.h"

Entity::Entity(std::shared_ptr<Mesh> mesh,
    std::shared_ptr<Texture2D> texture,
    const glm::vec3& pos = { 0.0f, 0.0f, 0.0f },
    const glm::vec3& eulerRot = { 0.0f, 0.0f, 0.0f },
    const glm::vec3& scale = { 1.0f, 1.0f, 1.0f }) : mesh(mesh), texture(texture), pos(pos), eulerRot(eulerRot), scale(scale) {}

std::shared_ptr<Texture2D> Entity::lastTexture{};
ProgramObject* Entity::lastProgram = nullptr;

glm::mat4 Entity::getLocalModelMatrix() const {
    const glm::mat4 transformX = glm::rotate(glm::mat4(1.0f),
        glm::radians(eulerRot.x),
        glm::vec3(1.0f, 0.0f, 0.0f));
    const glm::mat4 transformY = glm::rotate(glm::mat4(1.0f),
        glm::radians(eulerRot.y),
        glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 transformZ = glm::rotate(glm::mat4(1.0f),
        glm::radians(eulerRot.z),
        glm::vec3(0.0f, 0.0f, 1.0f));

    const glm::mat4 roationMatrix = transformY * transformX * transformZ;

    return glm::translate(glm::mat4(1.0f), pos) *
        roationMatrix *
        glm::scale(glm::mat4(1.0f), scale);
}

void Entity::GenerateReflection(bool _generateReflection) {
    if (_generateReflection) {
        glm::vec3 center;
        float radius;

        mesh->getBoundingSphere(center, radius);
        center += pos;
        //radius *= std::max(std::max(scale.x, scale.y), scale.z);

        environmentMap = std::make_unique<EnvironmentMap>(center, radius);
    } else {
        environmentMap.reset();
    }
}

bool Entity::getGenerateReflection() const {
    return environmentMap.get() != nullptr;
}

void Entity::update(const std::vector<Entity>& entities) {
    bool lastVal = false;
    if (reflected)lastVal = true;
    if (getGenerateReflection()) {
        environmentMap->UpdateScene(entities);
    }
    reflected = lastVal;
}

void Entity::setTexture(ProgramObject& program) const{
    if (&program != lastProgram) {
        lastProgram = &program;
        lastTexture.reset();
    }

    if (lastTexture.get() != texture.get()) {
        lastTexture = texture;
        program.SetTexture("texImage", 0, *texture);
    }
    program.SetTexture("texImage", 0, *texture);
}

void Entity::Reset() {
    lastProgram = nullptr;
}

void Entity::moved() {
    if (getGenerateReflection()) {
        glm::vec3 center;
        float radius;

        mesh->getBoundingSphere(center, radius);
        center += pos;
        //radius *= std::max(std::max(scale.x, scale.y), scale.z);

        environmentMap->updatePosition(center, radius);
    }
}