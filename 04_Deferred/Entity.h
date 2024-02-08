#ifndef ENTITY__H
#define ENTITY__H

#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Includes/Mesh_OGL3.h"
#include "Includes/TextureObject.h"
#include "EnvironmentMap.h"
#include "Includes/ProgramObject.h"

class Entity {
    static std::shared_ptr<Texture2D> lastTexture;
    static ProgramObject* lastProgram;
public:
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Texture2D> texture;

    glm::vec3 pos;
    glm::vec3 eulerRot;
    glm::vec3 scale;
    bool castShadow = true;
    bool receiveShadow = true;

    std::unique_ptr<EnvironmentMap> environmentMap;
    bool reflected = true;

    Entity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture2D> texture, const glm::vec3& pos, const glm::vec3& eulerRot, const glm::vec3& scale);
    void GenerateReflection(bool reflection);

    glm::mat4 getLocalModelMatrix() const;
    bool getGenerateReflection() const;
    void update(const std::vector<Entity>& entities);
    void setTexture(ProgramObject& program) const;
    void moved();

    static void Reset();
};

#endif // !ENTITY__H
