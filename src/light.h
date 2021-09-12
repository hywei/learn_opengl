#pragma once

#include <glm/glm.hpp>

class Light {
public:
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

class DirectionalLight : public Light {

public:
    glm::vec3 direction;
};

class PointLight : public Light {
public:
    glm::vec3 position;

    float constant {0.f};
    float linear {0.f};
    float quadratic {0.f};
};

class SpotLight : public Light {

public:
    glm::vec3 position;
    glm::vec3 direction;

    float cutoff {0.f};
};