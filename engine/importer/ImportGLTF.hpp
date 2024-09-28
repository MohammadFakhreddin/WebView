#pragma once

#include <filesystem>

#include "AssetGLTF_Mesh.hpp"
#include "AssetGLTF_Model.hpp"

#include <memory>
#include <string>

namespace MFA::Importer
{

    using Model = AS::GLTF::Model;
    using Mesh = AS::GLTF::Mesh;
    using Vertex = AS::GLTF::Vertex;
    using Index = AS::GLTF::Index;
    using Primitive = AS::GLTF::Primitive;
    using Animation = AS::GLTF::Animation;
    using Node = AS::GLTF::Node;
    using Skin = AS::GLTF::Skin;

    std::shared_ptr<Model> GLTF_Model(std::string const& path);
}
