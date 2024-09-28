#include "AssetGLTF_Mesh.hpp"

#include "BedrockAssert.hpp"
#include "BedrockMath.hpp"

namespace MFA::Asset::GLTF
{

    //-------------------------------------------------------------------------------------------------

    Node::Node() = default;

    //-------------------------------------------------------------------------------------------------

	bool Node::hasSubMesh() const noexcept
	{
        return subMeshIndex >= 0;
	}

    //-------------------------------------------------------------------------------------------------

	bool Node::HasParent() const noexcept
	{
        return parent >= 0;
	}

    //-------------------------------------------------------------------------------------------------

	Mesh::Mesh(
		uint32_t const vertexCount,
		uint32_t const indexCount,
		std::shared_ptr<Blob> vertexBuffer,
		std::shared_ptr<Blob> indexBuffer
	)
	{
		mVertexCount = vertexCount;

		mIndexCount = indexCount;

		MFA_ASSERT(vertexBuffer->IsValid() == true);
		mVertexData = std::move(vertexBuffer);

		MFA_ASSERT(indexBuffer->IsValid() == true);
		mIndexData = std::move(indexBuffer);

		mData = std::make_shared<MeshData>();

		mIndicesStartingIndex = 0;
		mVerticesStartingIndex = 0;

		mNextVertexOffset = 0;
		mNextIndexOffset = 0;
	}

	//-------------------------------------------------------------------------------------------------

	Mesh::~Mesh() = default;

	//-------------------------------------------------------------------------------------------------

	void Mesh::FinalizeData()
	{
        MFA_ASSERT(mNextIndexOffset == mIndexData->Len());
        MFA_ASSERT(mNextVertexOffset == mVertexData->Len());

        // Creating position min max for entire mesh based on subMeshes
        for (auto& subMesh : mData->subMeshes)
        {
            for (auto& primitive : subMesh.primitives)
            {
                switch (primitive.alphaMode)
                {
                case AlphaMode::Opaque:
                    subMesh.opaquePrimitives.emplace_back(&primitive);
                    break;
                case AlphaMode::Blend:
                    subMesh.blendPrimitives.emplace_back(&primitive);
                    break;
                case AlphaMode::Mask:
                    subMesh.maskPrimitives.emplace_back(&primitive);
                    break;
                default:
                    MFA_LOG_ERROR("Unhandled primitive alpha mode detected %d", static_cast<int>(primitive.alphaMode));
                    break;
                }
            }
        }

        MFA_ASSERT(mData->nodes.empty() == false);
        MFA_ASSERT(mData->rootNodes.empty() == true);

        // Step one: Store parent index for each child
        for (int i = 0; i < static_cast<int>(mData->nodes.size()); ++i)
        {
            auto const& currentNode = mData->nodes[i];
            if (currentNode.children.empty() == false)
            {
                for (auto const child : currentNode.children)
                {
                    mData->nodes[child].parent = i;
                }
            }
        }
        // Step two: Cache parent nodes in a separate daa structure for faster access
        for (uint32_t i = 0; i < static_cast<uint32_t>(mData->nodes.size()); ++i)
        {
            auto const& currentNode = mData->nodes[i];
            if (currentNode.parent < 0)
            {
                mData->rootNodes.emplace_back(i);
            }
        }
	}

	//-------------------------------------------------------------------------------------------------

	uint32_t Mesh::InsertSubMesh() const
	{
		mData->subMeshes.emplace_back();
		return static_cast<uint32_t>(mData->subMeshes.size() - 1);
	}

	//-------------------------------------------------------------------------------------------------

	void Mesh::InsertPrimitive(
        uint32_t subMeshIndex,
        Primitive primitive,
        uint32_t vertexCount,
        Vertex* vertices,
        uint32_t indicesCount,
        Index* indices
	)
	{
        MFA_ASSERT(vertexCount > 0);
        MFA_ASSERT(indicesCount > 0);
        MFA_ASSERT(vertices != nullptr);
        MFA_ASSERT(indices != nullptr);

		primitive.vertexCount = vertexCount;
        primitive.indicesCount = indicesCount;
        primitive.indicesOffset = mNextIndexOffset;
        primitive.verticesOffset = mNextVertexOffset;
        primitive.indicesStartingIndex = mIndicesStartingIndex;
        primitive.verticesStartingIndex = mVerticesStartingIndex;
        uint32_t const verticesSize = sizeof(Vertex) * vertexCount;
        uint32_t const indicesSize = sizeof(Index) * indicesCount;

		MFA_ASSERT(mNextVertexOffset + verticesSize <= mVertexData->Len());
        MFA_ASSERT(mNextIndexOffset + indicesSize <= mIndexData->Len());

        ::memcpy(mVertexData->Ptr() + mNextVertexOffset, vertices, verticesSize);
        ::memcpy(mIndexData->Ptr() + mNextIndexOffset, indices, indicesSize);

        MFA_ASSERT(subMeshIndex < mData->subMeshes.size());
        auto& subMesh = mData->subMeshes[subMeshIndex];

        if (primitive.hasPositionMinMax)
        {
            subMesh.hasPositionMinMax = true;
            // Position min
            if (primitive.positionMin[0] < subMesh.positionMin[0])
            {
                subMesh.positionMin[0] = primitive.positionMin[0];
            }
            if (primitive.positionMin[1] < subMesh.positionMin[1])
            {
                subMesh.positionMin[1] = primitive.positionMin[1];
            }
            if (primitive.positionMin[2] < subMesh.positionMin[2])
            {
                subMesh.positionMin[2] = primitive.positionMin[2];
            }

            // Position max
            if (primitive.positionMax[0] > subMesh.positionMax[0])
            {
                subMesh.positionMax[0] = primitive.positionMax[0];
            }
            if (primitive.positionMax[1] > subMesh.positionMax[1])
            {
                subMesh.positionMax[1] = primitive.positionMax[1];
            }
            if (primitive.positionMax[2] > subMesh.positionMax[2])
            {
                subMesh.positionMax[2] = primitive.positionMax[2];
            }
        }
        subMesh.primitives.emplace_back(primitive);

        mNextVertexOffset += verticesSize;
        mNextIndexOffset += indicesSize;
        mIndicesStartingIndex += indicesCount;
        mVerticesStartingIndex += vertexCount;
	}

    //-------------------------------------------------------------------------------------------------

    Node & Mesh::InsertNode() const
    {
        mData->nodes.emplace_back();
        return mData->nodes.back();
    }

    //-------------------------------------------------------------------------------------------------

    Skin & Mesh::InsertSkin() const
    {
        mData->skins.emplace_back();
        return mData->skins.back();
    }
    
    //-------------------------------------------------------------------------------------------------

    void Mesh::InsertAnimation(Animation const & animation) const
    {
        mData->animations.emplace_back(animation);
    }

	//-------------------------------------------------------------------------------------------------

	bool Mesh::IsValid() const
	{
		return mVertexCount > 0 &&
			mVertexData->IsValid() &&
			mIndexCount > 0 &&
			mIndexData->IsValid();
	}

	//-------------------------------------------------------------------------------------------------

	std::shared_ptr<MeshData> const& Mesh::GetMeshData() const
	{
        return mData;
	}

	//-------------------------------------------------------------------------------------------------

	uint32_t Mesh::GetVertexCount() const
	{
        return mVertexCount;
	}

	//-------------------------------------------------------------------------------------------------

	std::shared_ptr<Blob> const& Mesh::GetVertexData() const
	{
        return mVertexData;
	}

	//-------------------------------------------------------------------------------------------------

	uint32_t Mesh::GetIndexCount() const
	{
        return mIndexCount;
	}

	//-------------------------------------------------------------------------------------------------

	std::shared_ptr<Blob> const& Mesh::GetIndexData() const
	{
        return mIndexData;
	}

	//-------------------------------------------------------------------------------------------------

    void Mesh::CenterMesh()
    {
        auto * vertices = mVertexData->As<Vertex>();
        
        glm::vec3 minimum{};
		glm::vec3 maximum{};
		bool minMaxSetOnce = false;

		for (int i = 0; i < mVertexCount; ++i)
		{
            auto const & vertex = vertices[i];

			if (minMaxSetOnce == false)
			{
				minMaxSetOnce = true;
				minimum = vertex.position;
				maximum = vertex.position;
			}
			else
			{
				if (minimum.x > vertex.position.x)
				{
					minimum.x = vertex.position.x;
				}
				else if (maximum.x < vertex.position.x)
				{
					maximum.x = vertex.position.x;
				}

				if (minimum.y > vertex.position.y)
				{
					minimum.y = vertex.position.y;
				}
				else if (maximum.y < vertex.position.y)
				{
					maximum.y = vertex.position.y;
				}

				if (minimum.z > vertex.position.z)
				{
					minimum.z = vertex.position.z;
				}
				else if (maximum.z < vertex.position.z)
				{
					maximum.z = vertex.position.z;
				}
			}
		}

		glm::vec3 const center = (maximum + minimum) * 0.5f;

		for (int i = 0; i < mVertexCount; ++i)
		{
			vertices[i].position -= center;
		}

        mIsCentered = true;
    }

	//-------------------------------------------------------------------------------------------------

    void Mesh::Optimize()
    {
        auto * vertices = mVertexData->As<Vertex>();
        auto * indices = mIndexData->As<Index>();

        std::vector<Vertex> vertices2 {};

        for (int i = 0; i < mVertexCount; ++i)
        {
            bool isDuplicate = false;
            int nIdx = i;

            for (int j = 0; j < vertices2.size(); ++j)
            {
                if (glm::length2(vertices2[j].position - vertices[i].position) < glm::epsilon<float>())
                {
                    isDuplicate = true;
                    nIdx = j;
                    break;
                }
            }

            if (isDuplicate == false)
            {
                nIdx = vertices2.size();
                vertices2.emplace_back(vertices[i]);
            }

            for (int k = 0; k < mIndexCount; ++k)
            {
                if (indices[k] == i)
                {
                    indices[k] = nIdx;
                }
            }

        }

        mVertexData = Memory::Alloc(vertices2.data(), vertices2.size());
        mVertexCount = vertices2.size();

        std::vector<Index> indices2{};

        for (int i = 0; i < mIndexCount / 3; ++i)
        {

            auto const idV1x0 = indices[i * 3];
            auto const idV1x1 = indices[i * 3 + 1];
            auto const idV1x2 = indices[i * 3 + 2];

            bool isDuplicate = false;
            for (int j = 0; j < indices2.size() / 3; ++j)
            {
                auto const idV2x0 = indices2[j * 3];
                auto const idV2x1 = indices2[j * 3 + 1];
                auto const idV2x2 = indices2[j * 3 + 2];

                MFA_ASSERT(idV2x0 != idV2x1);
                MFA_ASSERT(idV2x0 != idV2x2);
                MFA_ASSERT(idV2x1 != idV2x2);

                if (idV1x0 == idV2x0 || 
                    idV1x0 == idV2x1 ||
                    idV1x0 == idV2x2)
                {
                    if (idV1x1 == idV2x0 || 
                        idV1x1 == idV2x1 ||
                        idV1x1 == idV2x2)
                    {
                        if (idV1x2 == idV2x0 || 
                            idV1x2 == idV2x1 ||
                            idV1x2 == idV2x2)
                        {
                            isDuplicate = true;    
                            break;
                        }
                    }
                }
            }
            if (isDuplicate == false)
            {
                indices2.emplace_back(idV1x0);
                indices2.emplace_back(idV1x1);
                indices2.emplace_back(idV1x2);
            }
        }

        mIndexData = Memory::Alloc(indices2.data(), indices2.size());
        mIndexCount = indices2.size();

        mIsOptimized = true;
    }

    //-------------------------------------------------------------------------------------------------

    bool Mesh::IsCentered() const noexcept
    {
	    return mIsCentered;
    }

    //-------------------------------------------------------------------------------------------------

    bool Mesh::IsOptimized() const noexcept
    {
	    return mIsOptimized;
    }

    //-------------------------------------------------------------------------------------------------

}
