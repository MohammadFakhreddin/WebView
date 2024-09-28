#pragma once

#include <string>
#include <vec2.hpp>
#include <vec3.hpp>
#include <vector>

namespace MFA::Importer
{

    struct Vertex
    {
        glm::vec3 position{};
        glm::vec3 normal{};
        glm::vec2 uv{};
    };

    struct ObjModel
    {
        std::vector<Vertex> vertices{};
        std::vector<int> indices{};
        bool hasNormals = true;
        bool hasTexCoords = true;
    };

    bool LoadObj(std::string const & path, ObjModel & outModel);
}
