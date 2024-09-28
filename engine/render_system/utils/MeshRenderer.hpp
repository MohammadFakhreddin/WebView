#pragma once

#include "pipeline/FlatShadingPipeline.hpp"
#include "RenderBackend.hpp"
#include "RenderTypes.hpp"
#include "ImportGLTF.hpp"

#include <memory>

namespace MFA
{

    class MeshInstance;

    class MeshRenderer
    {
    public:

        explicit MeshRenderer(
            std::shared_ptr<FlatShadingPipeline> pipeline,
            std::shared_ptr<AS::GLTF::Model> const& model,
            std::shared_ptr<RT::GpuTexture> errorTexture,
            bool hasOverrideColor = false,
            glm::vec4 overrideColor = {}
        );

        void Render(RT::CommandRecordState& recordState, std::vector<glm::mat4> const& models);

        void Render(RT::CommandRecordState& recordState, std::vector<MeshInstance*> const& instances) const;
        
        [[nodiscard]]
        std::vector<glm::vec3> GetVertices(glm::mat4 const& model) const noexcept;

        std::vector<std::vector<std::tuple<int, int>>> GetNeighbors() const noexcept;

        std::vector<Asset::GLTF::Node> const & GetNodes() const noexcept;

    private:

        std::shared_ptr<RT::BufferGroup> GenerateVertexBuffer(VkCommandBuffer cb, AS::GLTF::Model const& model);

        std::shared_ptr<RT::BufferGroup> GenerateIndexBuffer(VkCommandBuffer cb, AS::GLTF::Model const& model);

        std::vector<std::shared_ptr<RT::BufferAndMemory>> GenerateTextures(VkCommandBuffer cb, AS::GLTF::Model const& model);

        std::vector<std::shared_ptr<RT::BufferGroup>> CreateMaterials(VkCommandBuffer cb);

        void CreateDescriptorSets();
        
        void DrawSubMesh(
            RT::CommandRecordState& recordState,
            int subMeshIdx,
            glm::mat4 const & transform
        ) const;

        void DrawNode(
            RT::CommandRecordState& recordState,
            Asset::GLTF::Node & node,
            glm::mat4 const & model
        ) const;

        void DrawNode(
            RT::CommandRecordState& recordState,
            Asset::GLTF::Node & node,
            std::vector<Asset::GLTF::Node> & nodes
        ) const;

        std::shared_ptr<FlatShadingPipeline> _pipeline{};

        std::shared_ptr<AS::GLTF::MeshData> _meshData{};

        std::shared_ptr<RT::GpuTexture> _errorTexture{};

        std::shared_ptr<RT::BufferAndMemory> _verticesBuffer{};
        std::shared_ptr<RT::BufferAndMemory> _indicesBuffer{};
        std::vector<std::shared_ptr<RT::GpuTexture>> _textures{};
        std::vector<std::shared_ptr<RT::BufferGroup>> _materials{};
        std::vector<std::vector<RT::DescriptorSetGroup>> _descriptorSets;
        
        int _vertexCount{};
        std::shared_ptr<Blob> _vertices{};

        int _indexCount{};
        std::shared_ptr<Blob> _indices{};

        bool _hasOverrideColor{};
        glm::vec4 _overrideColor{};
    };

}
