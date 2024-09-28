#include "ImportObj.hpp"

#include <fstream>
#include <string>

#include "BedrockLog.hpp"
#include "tiny_gltf_loader/tiny_gltf_loader.h"
#include "tiny_obj_loader/tiny_obj_loader.h"

namespace MFA::Importer
{
    bool LoadObj(std::string const& path, ObjModel& outModel)
    {

        /*std::ifstream infile(path);
        if (infile.good() ) {
            return false;
        }*/

        if (path.empty()) { // Fail if model file could not be found:
            return false;
        }

        std::ifstream ifs(path);
        std::string content(
            (std::istreambuf_iterator<char>(ifs)),
            (std::istreambuf_iterator<char>())
        );

        tinyobj::attrib_t attributes;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warning;
        std::string error;
        std::istringstream sourceStream(content);
        if (!tinyobj::LoadObj(&attributes, &shapes, &materials, &warning, &error, &sourceStream)) {
            return false;
        }

        outModel.vertices.clear();
        outModel.indices.clear();

        for (const auto & shape : shapes) 
        {
            std::map<std::tuple<int, int, int>, uint32_t> uniqueVertices;

            bool hasVertices = attributes.vertices.empty() == false;
            bool hasTexCoords = attributes.texcoords.empty() == false;
            bool hasNormals = attributes.normals.empty() == false;

            outModel.hasTexCoords = hasTexCoords;
            outModel.hasNormals = hasNormals;

            for (const auto& indices : shape.mesh.indices) {
                glm::vec3 pos = hasVertices ? glm::vec3(
                    attributes.vertices[3 * indices.vertex_index],
                    attributes.vertices[3 * indices.vertex_index + 1],
                    attributes.vertices[3 * indices.vertex_index + 2]
                ) : glm::vec3{};
                glm::vec2 uv = hasTexCoords ? glm::vec2(
                    attributes.texcoords[2 * indices.texcoord_index],
                    1.0f - attributes.texcoords[2 * indices.texcoord_index + 1]
                ) : glm::vec2{};
                glm::vec3 normal = hasNormals ? glm::vec3(
                    attributes.normals[3 * indices.normal_index],
                    attributes.normals[3 * indices.normal_index + 1],
                    attributes.normals[3 * indices.normal_index + 2]
                ) : glm::vec3{};

                std::tuple<int, int, int> tuple = std::tuple<int, int, int>
                    (indices.vertex_index, indices.normal_index, indices.texcoord_index);

                if (uniqueVertices.find(tuple) == uniqueVertices.end()) {
                    uniqueVertices.insert({ tuple, static_cast<uint32_t>(outModel.vertices.size()) });

                    outModel.vertices.emplace_back();
                    auto & vertex = outModel.vertices.back();
                    vertex.position = pos;
                    vertex.uv = uv;
                    vertex.normal = normal;
                }
                outModel.indices.push_back(uniqueVertices[tuple]);
            }
        }
        return true;
    }
}
