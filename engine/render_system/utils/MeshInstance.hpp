#pragma once

#include "AssetGLTF_Mesh.hpp"
#include "Transform.hpp"

namespace MFA
{

    class MeshRenderer;

    class MeshInstance
    {
    public:
        // TODO: Handle caching for parent and child
        using Node = Asset::GLTF::Node;

        explicit MeshInstance();

        explicit MeshInstance(MeshRenderer const & meshRenderer);

        [[nodiscard]]
        Node* FindNode(std::string const & name);

        [[nodiscard]]
        std::vector<Asset::GLTF::Node> & GetNodes();

        [[nodiscard]]
        Transform & GetTransform();

        void SetTransform(const Transform& transform);

    private:

        Transform _transform{};

        std::vector<Node> _nodes{};
    
    };
}
