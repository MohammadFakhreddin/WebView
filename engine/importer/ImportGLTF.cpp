#include "ImportGLTF.hpp"

#include "AssetTexture.hpp"
#include "ImportTexture.hpp"
#include "BedrockAssert.hpp"
#include "BedrockMath.hpp"

#include "json.hpp"
#include "stb_image.h"
#include "stb_image_write.h"
#include "tiny_gltf_loader.h"

namespace MFA::Importer
{

    struct TextureRef
    {
        std::string const gltfName{};
        uint8_t const index = 0;
        std::string const relativePath{};
    };

    //-------------------------------------------------------------------------------------------------

    static void GLTF_extractTextures(
        std::string const& path,
        tinygltf::Model const& gltfModel,
        std::vector<TextureRef>& outTextureRefs
    )
    {
        std::string directoryPath = std::filesystem::path(path).parent_path().string();

        // Extracting textures
        if (false == gltfModel.textures.empty())
        {
            for (auto const& texture : gltfModel.textures)
            {
                auto const& image = gltfModel.images[texture.source];

                std::string const imagePath = directoryPath + "/" + image.uri;

                TextureRef textureRef{
                    .gltfName = image.uri,
                    .index = static_cast<uint8_t>(outTextureRefs.size()),
                    .relativePath = imagePath
                };
                outTextureRefs.emplace_back(textureRef);
            }
        }

    }

    //-------------------------------------------------------------------------------------------------

    static int16_t GLTF_findTextureByName(
        char const* textureName,
        std::vector<TextureRef> const& textureRefs
    )
    {
        MFA_ASSERT(textureName != nullptr);
        if (false == textureRefs.empty())
        {
            for (auto const& textureRef : textureRefs)
            {
                if (textureRef.gltfName == textureName)
                {
                    return textureRef.index;
                }
            }
        }
        return -1;
        //MFA_CRASH("Image not found: %s", gltf_name);
    }


    //-------------------------------------------------------------------------------------------------

#define extractTextureAndUV_Index(gltfModel, textureInfo, textureRefs, outTextureIndex, outUV_Index)    \
    if (textureInfo.index >= 0)                                                                             \
    {                                                                                                       \
        auto const & emissive_texture = gltfModel.textures[textureInfo.index];                              \
        auto const & image = gltfModel.images[emissive_texture.source];                                     \
        outTextureIndex = GLTF_findTextureByName(image.uri.c_str(), textureRefs);                           \
        if (outTextureIndex >= 0)                                                                           \
        {                                                                                                   \
            outUV_Index = static_cast<uint16_t>(textureInfo.texCoord);                                      \
        }                                                                                                   \
    }

    //-------------------------------------------------------------------------------------------------

    static void copyDataIntoVertexMember(
        float* vertexMember,
        uint8_t const componentCount,
        float const* items,
        uint32_t const dataIndex
    )
    {
        Memory::Copy(vertexMember, &items[dataIndex * componentCount], componentCount);
    }

    //-------------------------------------------------------------------------------------------------

    static bool GLTF_extractPrimitiveDataAndTypeFromBuffer(
        tinygltf::Model const& gltfModel,
        tinygltf::Primitive& primitive,
        char const* fieldKey,
        int& outType,
        int& outComponentType,
        void const*& outData,
        uint32_t& outDataCount
    )
    {
        bool success = false;
        auto const findAttributeResult = primitive.attributes.find(fieldKey);
        if (findAttributeResult != primitive.attributes.end())
        {// Positions
            success = true;
            auto const attributeValue = findAttributeResult->second;
            MFA_REQUIRE(static_cast<size_t>(attributeValue) < gltfModel.accessors.size());
            auto const& accessor = gltfModel.accessors[attributeValue];
            outType = accessor.type;
            outComponentType = accessor.componentType;
            auto const& bufferView = gltfModel.bufferViews[accessor.bufferView];
            MFA_REQUIRE(bufferView.buffer < gltfModel.buffers.size());
            auto const& buffer = gltfModel.buffers[bufferView.buffer];
            outData = reinterpret_cast<void const*>(
                &buffer.data[bufferView.byteOffset + accessor.byteOffset]
                );
            outDataCount = static_cast<uint32_t>(accessor.count);
        }
        return success;
    }

    //-------------------------------------------------------------------------------------------------

    template<typename ItemType>
    static bool GLTF_extractPrimitiveDataFromBuffer(
        tinygltf::Model& gltfModel,
        tinygltf::Primitive& primitive,
        char const* fieldKey,
        int expectedComponentType,
        ItemType const*& outData,
        uint32_t& outDataCount
    )
    {
        bool success = false;
        if (primitive.attributes.find(fieldKey) != primitive.attributes.end())
        {// Positions
            success = true;
            MFA_REQUIRE(static_cast<size_t>(primitive.attributes[fieldKey]) < gltfModel.accessors.size());
            auto const& accessor = gltfModel.accessors[primitive.attributes[fieldKey]];
            MFA_ASSERT(accessor.componentType == expectedComponentType);
            auto const& bufferView = gltfModel.bufferViews[accessor.bufferView];
            MFA_REQUIRE(static_cast<size_t>(bufferView.buffer) < gltfModel.buffers.size());
            auto const& buffer = gltfModel.buffers[bufferView.buffer];
            outData = reinterpret_cast<ItemType const*>(
                &buffer.data[bufferView.byteOffset + accessor.byteOffset]
                );
            outDataCount = static_cast<uint32_t>(accessor.count);
        }
        return success;
    }

    //-------------------------------------------------------------------------------------------------

    static void GLTF_extractNodes(
        tinygltf::Model const & gltfModel,
        Mesh * mesh
    )
    {
        // Step3: Fill nodes
        if (false == gltfModel.nodes.empty())
        {
            MFA_ASSERT(mesh != nullptr);
            for (auto const & gltfNode : gltfModel.nodes)
            {
                Node & node = mesh->InsertNode();
                node.name = gltfNode.name;
                node.subMeshIndex = gltfNode.mesh;
                node.children = gltfNode.children;
                node.skin = gltfNode.skin;

                if (gltfNode.translation.empty() == false)
                {
                    MFA_ASSERT(gltfNode.translation.size() == 3);
                    glm::dvec3 translate{};
                    Memory::Copy<3>(translate, gltfNode.translation.data());
                    node.transform.SetLocalPosition(translate);
                }

                if (gltfNode.rotation.empty() == false)
                {
                    MFA_ASSERT(gltfNode.rotation.size() == 4);
                    glm::dquat rotation{};
                	Memory::Copy<4>(rotation, gltfNode.rotation.data());
                    node.transform.SetLocalQuaternion(rotation);
                }

                if (gltfNode.scale.empty() == false)
                {
                    MFA_ASSERT(gltfNode.scale.size() == 3);
                    glm::dvec3 scale{};
                	Memory::Copy<3>(scale, gltfNode.scale.data());
                    node.transform.SetLocalScale(scale);
                }

                if (gltfNode.matrix.empty() == false)
                {
                    glm::dmat4 extraTransform{};
                    Memory::Copy<16>(extraTransform, gltfNode.matrix.data());
                    node.transform.SetLocalExtraTransform(extraTransform);
                }
            }
        }
    }
    
    //-------------------------------------------------------------------------------------------------
    
    template<typename ItemType>
    static void GLTF_extractDataFromBuffer(
        tinygltf::Model & gltfModel,
        int const accessorIndex,
        int const expectedComponentType,
        ItemType const *& outData,
        uint32_t & outDataCount
    )
    {
        MFA_ASSERT(accessorIndex >= 0);
        auto const & accessor = gltfModel.accessors[accessorIndex];
        MFA_ASSERT(accessor.componentType == expectedComponentType);
        auto const & bufferView = gltfModel.bufferViews[accessor.bufferView];
        MFA_REQUIRE(bufferView.buffer < gltfModel.buffers.size());
        auto const & buffer = gltfModel.buffers[bufferView.buffer];
        outData = reinterpret_cast<ItemType const *>(
            &buffer.data[bufferView.byteOffset + accessor.byteOffset]
        );
        outDataCount = static_cast<uint32_t>(accessor.count);
    }

    static void GLTF_extractDataAndTypeFromBuffer(
        tinygltf::Model const & gltfModel,
        int accessorIndex,
        int expectedComponentType,
        int & outType,
        void const *& outData,
        uint32_t & outDataCount
    )
    {
        MFA_ASSERT(accessorIndex >= 0);
        auto const & accessor = gltfModel.accessors[accessorIndex];
        outType = accessor.type;
        MFA_ASSERT(expectedComponentType == accessor.componentType);
        auto const & bufferView = gltfModel.bufferViews[accessor.bufferView];
        MFA_REQUIRE(bufferView.buffer < gltfModel.buffers.size());
        auto const & buffer = gltfModel.buffers[bufferView.buffer];
        outData = reinterpret_cast<void const *>(
            &buffer.data[bufferView.byteOffset + accessor.byteOffset]
        );
        outDataCount = static_cast<uint32_t>(accessor.count);
    }

    //-------------------------------------------------------------------------------------------------

    static void GLTF_extractSkins(
        tinygltf::Model & gltfModel,
        Mesh * mesh
    )
    {
        MFA_ASSERT(mesh != nullptr);

        for (auto const & gltfSkin : gltfModel.skins)
        {
            Skin & skin = mesh->InsertSkin();

            // Joints
            skin.joints.insert(skin.joints.end(), gltfSkin.joints.begin(), gltfSkin.joints.end());

            // InverseBindMatrices
            MFA_REQUIRE(gltfSkin.inverseBindMatrices >= 0);
            uint32_t inverseBindMatricesCount = 0;
            float const * inverseBindMatricesPtr = nullptr;
            GLTF_extractDataFromBuffer(
                gltfModel,
                gltfSkin.inverseBindMatrices,
                TINYGLTF_COMPONENT_TYPE_FLOAT,
                inverseBindMatricesPtr,
                inverseBindMatricesCount
            );
            MFA_ASSERT(inverseBindMatricesPtr != nullptr);
            skin.inverseBindMatrices.resize(inverseBindMatricesCount);
            for (size_t i = 0; i < skin.inverseBindMatrices.size(); ++i)
            {
                auto & currentMatrix = skin.inverseBindMatrices[i];
                Memory::Copy<16>(currentMatrix, &inverseBindMatricesPtr[i * 16]);
                //Matrix::CopyCellsToGlm(&inverseBindMatricesPtr[i * 16], currentMatrix);
            }
            MFA_ASSERT(skin.inverseBindMatrices.size() == skin.joints.size());
            skin.skeletonRootNode = gltfSkin.skeleton;
        }
    }

    //-------------------------------------------------------------------------------------------------

    static void GLTF_extractAnimations(
        tinygltf::Model & gltfModel,
        Mesh * mesh
    )
    {
        using Sampler = Animation::Sampler;
        using Channel = Animation::Channel;
        using Interpolation = Animation::Interpolation;
        using Path = Animation::Path;

        MFA_ASSERT(mesh != nullptr);

        auto const convertInterpolationToEnum = [](char const * value)-> Interpolation
        {
            if (strcmp("LINEAR", value) == 0)
            {
                return Interpolation::Linear;
            }
            if (strcmp("STEP", value) == 0)
            {
                return Interpolation::Step;
            }
            if (strcmp("CUBICSPLINE", value) == 0)
            {
                return Interpolation::CubicSpline;
            }
            MFA_CRASH("Unhandled test case");
            return Interpolation::Invalid;
        };

        auto const convertPathToEnum = [](char const * value)-> Path
        {
            if (strcmp("translation", value) == 0)
            {
                return Path::Translation;
            }
            if (strcmp("rotation", value) == 0)
            {
                return Path::Rotation;
            }
            if (strcmp("scale", value) == 0)
            {
                return Path::Scale;
            }
            MFA_CRASH("Unhandled test case");
            return Path::Invalid;
        };

        for (auto const & gltfAnimation : gltfModel.animations)
        {
            Animation animation{};
            animation.name = gltfAnimation.name;
            // Samplers
            for (auto const & gltfSampler : gltfAnimation.samplers)
            {
                Sampler sampler{};
                sampler.interpolation = convertInterpolationToEnum(gltfSampler.interpolation.c_str());

                {// Read sampler keyframe input time values
                    float const * inputData = nullptr;
                    uint32_t inputCount = 0;
                    GLTF_extractDataFromBuffer(
                        gltfModel,
                        gltfSampler.input,
                        TINYGLTF_COMPONENT_TYPE_FLOAT,
                        inputData,
                        inputCount
                    );
                    MFA_ASSERT(inputCount > 0);

                    sampler.inputAndOutput.resize(inputCount);
                    for (size_t index = 0; index < inputCount; index++)
                    {
                        auto const input = inputData[index];
                        sampler.inputAndOutput[index].input = input;
                        if (animation.startTime == -1.0f || animation.startTime > input)
                        {
                            animation.startTime = input;
                        }
                        if (animation.endTime == -1.0f || animation.endTime < input)
                        {
                            animation.endTime = input;
                        }
                    }
                }

                {// Read sampler keyframe output translate/rotate/scale values
                    void const * outputData = nullptr;
                    uint32_t outputCount = 0;
                    int outputDataType = 0;
                    GLTF_extractDataAndTypeFromBuffer(
                        gltfModel,
                        gltfSampler.output,
                        TINYGLTF_COMPONENT_TYPE_FLOAT,
                        outputDataType,
                        outputData,
                        outputCount
                    );
                    MFA_ASSERT(outputCount == sampler.inputAndOutput.size());

                    struct Output3
                    {
                        float value[3];
                    };

                    struct Output4
                    {
                        float value[4];
                    };

                    switch (outputDataType)
                    {
                    case TINYGLTF_TYPE_VEC3:
                    {
                        auto const * output = static_cast<Output3 const *>(outputData);
                        for (size_t index = 0; index < outputCount; index++)
                        {
                            Memory::Copy<3>(sampler.inputAndOutput[index].output, output[index].value);
                        }
                        break;
                    }
                    case TINYGLTF_TYPE_VEC4:
                    {
                        auto const * output = static_cast<Output4 const *>(outputData);
                        for (size_t index = 0; index < outputCount; index++)
                        {
                            Memory::Copy<4>(sampler.inputAndOutput[index].output, output[index].value);
                        }
                        break;
                    }
                    default:
                    {
                        MFA_REQUIRE(false);
                        break;
                    }
                    }
                }
                animation.samplers.emplace_back(sampler);
            }

            // Channels
            for (auto const & gltfChannel : gltfAnimation.channels)
            {
                Channel channel{};
                channel.path = convertPathToEnum(gltfChannel.target_path.c_str());
                channel.samplerIndex = gltfChannel.sampler;
                channel.nodeIndex = gltfChannel.target_node;
                animation.channels.emplace_back(channel);
            }
            
            mesh->InsertAnimation(animation);
        }
    }

    //-------------------------------------------------------------------------------------------------

    static std::shared_ptr<Mesh> GLTF_extractSubMeshes(
        tinygltf::Model& gltfModel,
        std::vector<TextureRef> const& textureRefs
    )
    {
        auto const generateUvKeyword = [](int32_t const uvIndex) -> std::string
        {
            return "TEXCOORD_" + std::to_string(uvIndex);
        };
        // Step1: Iterate over all meshes and gather required information for asset buffer
        uint32_t totalIndicesCount = 0;
        uint32_t totalVerticesCount = 0;
        uint32_t indicesVertexStartingIndex = 0;
        for (auto& mesh : gltfModel.meshes)
        {
            if (false == mesh.primitives.empty())
            {
                for (auto& primitive : mesh.primitives)
                {
                    {// Indices
                        MFA_REQUIRE((primitive.indices < gltfModel.accessors.size()));
                        auto const& accessor = gltfModel.accessors[primitive.indices];
                        totalIndicesCount += static_cast<uint32_t>(accessor.count);
                    }
                    {// Positions
                        MFA_REQUIRE((primitive.attributes["POSITION"] < gltfModel.accessors.size()));
                        auto const& accessor = gltfModel.accessors[primitive.attributes["POSITION"]];
                        totalVerticesCount += static_cast<uint32_t>(accessor.count);
                    }
                }
            }
        }

        auto mesh = std::make_shared<Mesh>(
            totalVerticesCount,
            totalIndicesCount,
            Memory::AllocSize(sizeof(Vertex) * totalVerticesCount),
            Memory::AllocSize(sizeof(Index) * totalIndicesCount)
        );

        // Step2: Fill subMeshes
        uint32_t primitiveUniqueId = 0;
        std::vector<Vertex> primitiveVertices{};
        std::vector<Index> primitiveIndices{};
        for (auto& gltfMesh : gltfModel.meshes)
        {
            auto const meshIndex = mesh->InsertSubMesh();
            if (false == gltfMesh.primitives.empty())
            {
                for (auto& gltfPrimitive : gltfMesh.primitives)
                {
                    primitiveIndices.erase(primitiveIndices.begin(), primitiveIndices.end());
                    primitiveVertices.erase(primitiveVertices.begin(), primitiveVertices.end());

                    int16_t baseColorTextureIndex = -1;
                    int32_t baseColorUvIndex = -1;
                    int16_t metallicRoughnessTextureIndex = -1;
                    int32_t metallicRoughnessUvIndex = -1;
                    int16_t normalTextureIndex = -1;
                    int32_t normalUvIndex = -1;
                    int16_t emissiveTextureIndex = -1;
                    int32_t emissiveUvIndex = -1;
                    int16_t occlusionTextureIndex = -1;
                    int32_t occlusionUV_Index = -1;
                    float baseColorFactor[4]{1.0f, 1.0f, 1.0f, 1.0f};
                    float metallicFactor = 0;
                    float roughnessFactor = 0;
                    float emissiveFactor[3]{};
                    bool doubleSided = false;
                    float alphaCutoff = 0.0f;

                    using AlphaMode = AS::GLTF::AlphaMode;
                    AlphaMode alphaMode = AlphaMode::Opaque;

                    uint32_t uniqueId = primitiveUniqueId;
                    primitiveUniqueId++;
                    if (gltfPrimitive.material >= 0)
                    {// Material
                        auto const& material = gltfModel.materials[gltfPrimitive.material];

                        // Base color texture
                        extractTextureAndUV_Index(
                            gltfModel,
                            material.pbrMetallicRoughness.baseColorTexture,
                            textureRefs,
                            baseColorTextureIndex,
                            baseColorUvIndex
                        );

                        // Metallic-roughness texture
                        extractTextureAndUV_Index(
                            gltfModel,
                            material.pbrMetallicRoughness.metallicRoughnessTexture,
                            textureRefs,
                            metallicRoughnessTextureIndex,
                            metallicRoughnessUvIndex
                        );

                        // Normal texture
                        extractTextureAndUV_Index(
                            gltfModel,
                            material.normalTexture,
                            textureRefs,
                            normalTextureIndex,
                            normalUvIndex
                        )

                            // Emissive texture
                            extractTextureAndUV_Index(
                                gltfModel,
                                material.emissiveTexture,
                                textureRefs,
                                emissiveTextureIndex,
                                emissiveUvIndex
                            )

                            // Occlusion texture
                            extractTextureAndUV_Index(
                                gltfModel,
                                material.occlusionTexture,
                                textureRefs,
                                occlusionTextureIndex,
                                occlusionUV_Index
                            )

                        {// BaseColorFactor
                            baseColorFactor[0] = static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[0]);
                            baseColorFactor[1] = static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[1]);
                            baseColorFactor[2] = static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[2]);
                            baseColorFactor[3] = static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[3]);
                        }
                        metallicFactor = static_cast<float>(material.pbrMetallicRoughness.metallicFactor);
                        roughnessFactor = static_cast<float>(material.pbrMetallicRoughness.roughnessFactor);
                        {// EmissiveFactor
                            emissiveFactor[0] = static_cast<float>(material.emissiveFactor[0]);
                            emissiveFactor[1] = static_cast<float>(material.emissiveFactor[1]);
                            emissiveFactor[2] = static_cast<float>(material.emissiveFactor[2]);
                        }

                        alphaCutoff = static_cast<float>(material.alphaCutoff);
                        alphaMode = [&material]()->AlphaMode
                        {
                            if (material.alphaMode == "OPAQUE")
                            {
                                return AlphaMode::Opaque;
                            }
                            if (material.alphaMode == "BLEND")
                            {
                                return AlphaMode::Blend;
                            }
                            if (material.alphaMode == "MASK")
                            {
                                return AlphaMode::Mask;
                            }
                            MFA_LOG_ERROR("Unhandled format detected: %s", material.alphaMode.c_str());
                            return AlphaMode::Invalid;
                        }();
                        doubleSided = material.doubleSided;
                    }

                    uint32_t primitiveIndicesCount = 0;
                    {// Indices
                        MFA_REQUIRE(gltfPrimitive.indices < gltfModel.accessors.size());
                        auto const& accessor = gltfModel.accessors[gltfPrimitive.indices];
                        auto const& bufferView = gltfModel.bufferViews[accessor.bufferView];
                        MFA_REQUIRE(bufferView.buffer < gltfModel.buffers.size());
                        auto const& buffer = gltfModel.buffers[bufferView.buffer];
                        primitiveIndicesCount = static_cast<uint32_t>(accessor.count);

                        switch (accessor.componentType)
                        {
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
                        {
                            auto const* gltfIndices = reinterpret_cast<uint32_t const*>(
                                &buffer.data[bufferView.byteOffset + accessor.byteOffset]
                                );
                            for (uint32_t i = 0; i < primitiveIndicesCount; i++)
                            {
                                primitiveIndices.emplace_back(gltfIndices[i] + indicesVertexStartingIndex);
                            }
                        }
                        break;
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
                        {
                            auto const* gltfIndices = reinterpret_cast<uint16_t const*>(
                                &buffer.data[bufferView.byteOffset + accessor.byteOffset]
                                );
                            for (uint32_t i = 0; i < primitiveIndicesCount; i++)
                            {
                                primitiveIndices.emplace_back(gltfIndices[i] + indicesVertexStartingIndex);
                            }
                        }
                        break;
                        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
                        {
                            auto const* gltfIndices = reinterpret_cast<uint8_t const*>(
                                &buffer.data[bufferView.byteOffset + accessor.byteOffset]
                                );
                            for (uint32_t i = 0; i < primitiveIndicesCount; i++)
                            {
                                primitiveIndices.emplace_back(gltfIndices[i] + indicesVertexStartingIndex);
                            }
                        }
                        break;
                        default:
                            MFA_NOT_IMPLEMENTED_YET("Mohammad Fakhreddin");
                        }
                    }

                    float const* positions = nullptr;
                    uint32_t primitiveVertexCount = 0;
                    float positionsMinValue[3]{};
                    float positionsMaxValue[3]{};
                    bool hasPositionMinMax = false;
                    {// Position
                        auto const result = GLTF_extractPrimitiveDataFromBuffer(
                            gltfModel,
                            gltfPrimitive,
                            "POSITION",
                            TINYGLTF_COMPONENT_TYPE_FLOAT,
                            positions,
                            primitiveVertexCount
                        );
                        MFA_ASSERT(result);
                    }

                    float const* baseColorUVs = nullptr;
                    float baseColorUV_Min[2]{};
                    float baseColorUV_Max[2]{};
                    bool hasBaseColorUvMinMax = false;
                    if (baseColorUvIndex >= 0)
                    {// BaseColor
                        uint32_t baseColorUvsCount = 0;
                        auto texture_coordinate_key_name = generateUvKeyword(baseColorUvIndex);
                        auto const result = GLTF_extractPrimitiveDataFromBuffer(
                            gltfModel,
                            gltfPrimitive,
                            texture_coordinate_key_name.c_str(),
                            TINYGLTF_COMPONENT_TYPE_FLOAT,
                            baseColorUVs,
                            baseColorUvsCount
                        );
                        MFA_ASSERT(result == true);
                        MFA_ASSERT(baseColorUvsCount == primitiveVertexCount);
                    }

                    float const* metallicRoughnessUvs = nullptr;
                    float metallicRoughnessUVMin[2]{};
                    float metallicRoughnessUVMax[2]{};
                    bool hasMetallicRoughnessUvMinMax = false;
                    if (metallicRoughnessUvIndex >= 0)
                    {// MetallicRoughness uvs
                        std::string texture_coordinate_key_name = generateUvKeyword(metallicRoughnessUvIndex);
                        uint32_t metallicRoughnessUvsCount = 0;
                        auto const result = GLTF_extractPrimitiveDataFromBuffer(
                            gltfModel,
                            gltfPrimitive,
                            texture_coordinate_key_name.c_str(),
                            TINYGLTF_COMPONENT_TYPE_FLOAT,
                            metallicRoughnessUvs,
                            metallicRoughnessUvsCount
                        );
                        MFA_ASSERT(result == true);
                        MFA_ASSERT(metallicRoughnessUvsCount == primitiveVertexCount);
                    }

                    float const* emissionUVs = nullptr;
                    float emissionUV_Min[2]{};
                    float emissionUV_Max[2]{};
                    bool hasEmissionUvMinMax = false;
                    if (emissiveUvIndex >= 0)
                    {// Emission uvs
                        std::string textureCoordinateKeyName = generateUvKeyword(emissiveUvIndex);
                        uint32_t emissionUvCount = 0;
                        auto const result = GLTF_extractPrimitiveDataFromBuffer(
                            gltfModel,
                            gltfPrimitive,
                            textureCoordinateKeyName.c_str(),
                            TINYGLTF_COMPONENT_TYPE_FLOAT,
                            emissionUVs,
                            emissionUvCount
                        );
                        MFA_ASSERT(result == true);
                        MFA_ASSERT(emissionUvCount == primitiveVertexCount);
                    }

                    float const* occlusionUVs = nullptr;
                    float occlusionUV_Min[2]{};
                    float occlusionUV_Max[2]{};
                    bool hasOcclusionUV_MinMax = false;
                    if (occlusionUV_Index >= 0)
                    {// Occlusion uvs
                        std::string textureCoordinateKeyName = generateUvKeyword(occlusionUV_Index);
                        uint32_t occlusionUV_Count = 0;
                        auto const result = GLTF_extractPrimitiveDataFromBuffer(
                            gltfModel,
                            gltfPrimitive,
                            textureCoordinateKeyName.c_str(),
                            TINYGLTF_COMPONENT_TYPE_FLOAT,
                            occlusionUVs,
                            occlusionUV_Count
                        );
                        MFA_ASSERT(result == true);
                        MFA_ASSERT(occlusionUV_Count == primitiveVertexCount);
                    }

                    float const* normalsUVs = nullptr;
                    float normalsUV_Min[2]{};
                    float normalsUV_Max[2]{};
                    bool hasNormalUvMinMax = false;
                    if (normalUvIndex >= 0)
                    {// Normal uvs
                        std::string texture_coordinate_key_name = generateUvKeyword(normalUvIndex);
                        uint32_t normalUvsCount = 0;
                        auto const result = GLTF_extractPrimitiveDataFromBuffer(
                            gltfModel,
                            gltfPrimitive,
                            texture_coordinate_key_name.c_str(),
                            TINYGLTF_COMPONENT_TYPE_FLOAT,
                            normalsUVs,
                            normalUvsCount
                        );
                        MFA_ASSERT(result == true);
                        MFA_ASSERT(normalUvsCount == primitiveVertexCount);
                    }
                    float const* normalValues = nullptr;
                    float normalsValuesMin[3]{};
                    float normalsValuesMax[3]{};
                    bool hasNormalValueMinMax = false;
                    {// Normal values
                        uint32_t normalValuesCount = 0;
                        auto const result = GLTF_extractPrimitiveDataFromBuffer(
                            gltfModel,
                            gltfPrimitive,
                            "NORMAL",
                            TINYGLTF_COMPONENT_TYPE_FLOAT,
                            normalValues,
                            normalValuesCount
                        );
                        MFA_ASSERT(result == false || normalValuesCount == primitiveVertexCount);
                    }

                    float const* tangentValues = nullptr;
                    float tangentsValuesMin[4]{};
                    float tangentsValuesMax[4]{};
                    bool hasTangentsValuesMinMax = false;
                    {// Tangent values
                        uint32_t tangentValuesCount = 0;
                        auto const result = GLTF_extractPrimitiveDataFromBuffer(
                            gltfModel,
                            gltfPrimitive,
                            "TANGENT",
                            TINYGLTF_COMPONENT_TYPE_FLOAT,
                            tangentValues,
                            tangentValuesCount
                        );
                        MFA_ASSERT(result == false || tangentValuesCount == primitiveVertexCount);
                    }


                    uint32_t jointItemCount = 0;
                    std::vector<uint16_t> jointValues{};
                    int jointAccessorType = 0;
                    {// Joints
                        void const* rawJointValues = nullptr;
                        int componentType = 0;
                        uint32_t jointValuesCount = 0;
                        GLTF_extractPrimitiveDataAndTypeFromBuffer(
                            gltfModel,
                            gltfPrimitive,
                            "JOINTS_0",
                            jointAccessorType,
                            componentType,
                            rawJointValues,
                            jointValuesCount
                        );
                        jointItemCount = jointValuesCount * jointAccessorType;
                        jointValues.resize(jointItemCount);
                        switch (componentType)
                        {
                        case 0:
                            break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                        {
                            auto const* shortJointValues = static_cast<uint16_t const*>(rawJointValues);
                            for (uint32_t i = 0; i < jointItemCount; ++i)
                            {
                                jointValues[i] = shortJointValues[i];
                            }
                        }
                        break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                        {
                            auto const* byteJointValues = static_cast<uint8_t const*>(rawJointValues);
                            for (uint32_t i = 0; i < jointItemCount; ++i)
                            {
                                jointValues[i] = byteJointValues[i];
                            }
                        }
                        break;
                        default:
                            MFA_CRASH("Unhandled type");
                        }
                        MFA_ASSERT((jointAccessorType > 0 || jointItemCount == 0));
                    }
                    std::vector<float> weightValues{};
                    {// Weights
                        int componentType = 0;
                        int accessorType = 0;
                        uint32_t rawValuesCount = 0;
                        void const* rawValues = nullptr;

                        GLTF_extractPrimitiveDataAndTypeFromBuffer(
                            gltfModel,
                            gltfPrimitive,
                            "WEIGHTS_0",
                            accessorType,
                            componentType,
                            rawValues,
                            rawValuesCount
                        );

                        auto itemCount = rawValuesCount * accessorType;

                        MFA_ASSERT(itemCount == jointItemCount);
                        MFA_ASSERT(accessorType == jointAccessorType);

                        weightValues.resize(itemCount);
                        switch (componentType)
                        {
                        case 0:
                            break;
                        case TINYGLTF_COMPONENT_TYPE_FLOAT:
                        {
                            auto const* floatWeightValues = static_cast<float const*>(rawValues);
                            for (uint32_t i = 0; i < jointItemCount; ++i)
                            {
                                weightValues[i] = floatWeightValues[i];
                            }
                        }
                        break;
                        default:
                            MFA_CRASH("Unhandled type");
                        }
                        MFA_ASSERT((accessorType > 0 || itemCount == 0));
                    }
                    // TODO Start from here, Assign weight and joint
                    float const* colors = nullptr;
                    float colorsMinValue[3]{ 0 };
                    float colorsMaxValue[3]{ 1 };
                    float colorsMinMaxDiff[3]{ 1 };
                    if (gltfPrimitive.attributes["COLOR"] >= 0)
                    {
                        MFA_REQUIRE(gltfPrimitive.attributes["COLOR"] < gltfModel.accessors.size());
                        auto const& accessor = gltfModel.accessors[gltfPrimitive.attributes["COLOR"]];
                        //MFA_ASSERT(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
                        //TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT
                        if (accessor.minValues.size() == 3 && accessor.maxValues.size() == 3)
                        {
                            colorsMinValue[0] = static_cast<float>(accessor.minValues[0]);
                            colorsMinValue[1] = static_cast<float>(accessor.minValues[1]);
                            colorsMinValue[2] = static_cast<float>(accessor.minValues[2]);
                            colorsMaxValue[0] = static_cast<float>(accessor.maxValues[0]);
                            colorsMaxValue[1] = static_cast<float>(accessor.maxValues[1]);
                            colorsMaxValue[2] = static_cast<float>(accessor.maxValues[2]);
                            colorsMinMaxDiff[0] = colorsMaxValue[0] - colorsMinValue[0];
                            colorsMinMaxDiff[1] = colorsMaxValue[1] - colorsMinValue[1];
                            colorsMinMaxDiff[2] = colorsMaxValue[2] - colorsMinValue[2];
                        }
                        auto const& bufferView = gltfModel.bufferViews[accessor.bufferView];
                        MFA_REQUIRE(bufferView.buffer < gltfModel.buffers.size());
                        auto const& buffer = gltfModel.buffers[bufferView.buffer];
                        colors = reinterpret_cast<const float*>(                           // TODO: Variable not used! Why?
                            &buffer.data[bufferView.byteOffset + accessor.byteOffset]
                            );
                    }

                    bool hasPosition = positions != nullptr;
                    MFA_ASSERT(hasPosition == true);
                    bool hasBaseColorTexture = baseColorUVs != nullptr;
                    MFA_ASSERT(baseColorUVs != nullptr == baseColorTextureIndex >= 0);
                    bool hasNormalValue = normalValues != nullptr;
                    MFA_ASSERT(hasNormalValue == true);
                    bool hasNormalTexture = normalsUVs != nullptr;
                    MFA_ASSERT(normalsUVs != nullptr == normalTextureIndex >= 0);
                    bool hasCombinedMetallicRoughness = metallicRoughnessUvs != nullptr;
                    MFA_ASSERT(metallicRoughnessUvs != nullptr == metallicRoughnessTextureIndex >= 0);
                    bool hasEmissiveTexture = emissionUVs != nullptr;
                    MFA_ASSERT(emissionUVs != nullptr == emissiveTextureIndex >= 0);
                    bool hasOcclusionTexture = occlusionUVs != nullptr;
                    MFA_ASSERT((occlusionUVs != nullptr) == (occlusionTextureIndex >= 0));
                    bool hasTangentValue = tangentValues != nullptr;
                    bool hasSkin = jointItemCount > 0;
                    for (uint32_t i = 0; i < primitiveVertexCount; ++i)
                    {
                        primitiveVertices.emplace_back();
                        auto& vertex = primitiveVertices.back();

                        // Positions
                        if (hasPosition)
                        {
                            copyDataIntoVertexMember(
                                &vertex.position[0],
                                3,
                                positions,
                                i
                            );
                        }

                        // Normal values
                        if (hasNormalValue)
                        {
                            copyDataIntoVertexMember(
                                &vertex.normal[0],
                                3,
                                normalValues,
                                i
                            );
                        }

                        // Normal uvs
                        if (hasNormalTexture)
                        {
                            copyDataIntoVertexMember(
                                &vertex.normalMapUV[0],
                                2,
                                normalsUVs,
                                i
                            );
                        }

                        if (hasTangentValue)
                        {// Tangent
                            copyDataIntoVertexMember(
                                &vertex.tangent[0],
                                4,
                                tangentValues,
                                i
                            );
                        }

                        if (hasEmissiveTexture)
                        {// Emissive
                            copyDataIntoVertexMember(
                                &vertex.emissionUV[0],
                                2,
                                emissionUVs,
                                i
                            );
                        }

                        if (hasBaseColorTexture)
                        {// BaseColor
                            copyDataIntoVertexMember(
                                &vertex.baseColorUV[0],
                                2,
                                baseColorUVs,
                                i
                            );
                        }

                        if (hasOcclusionTexture)
                        {// Occlusion
                            copyDataIntoVertexMember(
                                &vertex.occlusionUV[0],
                                2,
                                occlusionUVs,
                                i
                            );
                        }

                        if (hasCombinedMetallicRoughness)
                        {// MetallicRoughness
                            vertex.roughnessUV[0] = metallicRoughnessUvs[i * 2 + 0];
                            vertex.roughnessUV[1] = metallicRoughnessUvs[i * 2 + 1];
                            Memory::Copy(vertex.metallicUV, vertex.roughnessUV);
                            static_assert(sizeof(vertex.roughnessUV) == sizeof(vertex.metallicUV));
                            if (hasMetallicRoughnessUvMinMax)
                            {
                                MFA_ASSERT(vertex.roughnessUV[0] >= metallicRoughnessUVMin[0]);
                                MFA_ASSERT(vertex.roughnessUV[0] <= metallicRoughnessUVMax[0]);
                                MFA_ASSERT(vertex.roughnessUV[1] >= metallicRoughnessUVMin[1]);
                                MFA_ASSERT(vertex.roughnessUV[1] <= metallicRoughnessUVMax[1]);
                            }
                        }
                        // TODO WTF ? Outside of range error. Why do we need color range anyways ?
                        // vertex.color[0] = static_cast<uint8_t>((256/(colorsMinMaxDiff[0])) * colors[i * 3 + 0]);
                        // vertex.color[1] = static_cast<uint8_t>((256/(colorsMinMaxDiff[1])) * colors[i * 3 + 1]);
                        // vertex.color[2] = static_cast<uint8_t>((256/(colorsMinMaxDiff[2])) * colors[i * 3 + 2]);

                        vertex.hasSkin = hasSkin ? 1 : 0;

                        // Joint and weight
                        if (hasSkin)
                        {
                            for (int j = 0; j < jointAccessorType; j++)
                            {
                                vertex.jointIndices[j] = jointValues[i * jointAccessorType + j];
                                vertex.jointWeights[j] = weightValues[i * jointAccessorType + j];
                            }
                            for (int j = jointAccessorType; j < 4; j++)
                            {
                                vertex.jointIndices[j] = 0;
                                vertex.jointWeights[j] = 0;
                            }
                        }
                    }

                    {// Creating new subMesh
                        Primitive primitive{};
                        primitive.uniqueId = uniqueId;
                        primitive.baseColorTextureIndex = baseColorTextureIndex;
                        primitive.metallicRoughnessTextureIndex = metallicRoughnessTextureIndex;
                        primitive.normalTextureIndex = normalTextureIndex;
                        primitive.emissiveTextureIndex = emissiveTextureIndex;
                        primitive.occlusionTextureIndex = occlusionTextureIndex;
                        Memory::Copy(primitive.baseColorFactor, baseColorFactor);
                        primitive.metallicFactor = metallicFactor;
                        primitive.roughnessFactor = roughnessFactor;
                        Memory::Copy(primitive.emissiveFactor, emissiveFactor);
                        //primitive.occlusionStrengthFactor = occlusion
                        primitive.hasBaseColorTexture = hasBaseColorTexture;
                        primitive.hasEmissiveTexture = hasEmissiveTexture;
                        primitive.hasMetallicRoughnessTexture = hasCombinedMetallicRoughness;
                        primitive.hasNormalBuffer = hasNormalValue;
                        primitive.hasNormalTexture = hasNormalTexture;
                        primitive.hasTangentBuffer = hasTangentValue;
                        primitive.hasSkin = hasSkin;
                        primitive.hasPositionMinMax = hasPositionMinMax;
                        Memory::Copy(primitive.positionMin, positionsMinValue);
                        Memory::Copy(primitive.positionMax, positionsMaxValue);

                        primitive.alphaMode = alphaMode;
                        primitive.alphaCutoff = alphaCutoff;
                        primitive.doubleSided = doubleSided;

                        mesh->InsertPrimitive(
                            meshIndex,
                            primitive,
                            static_cast<uint32_t>(primitiveVertices.size()),
                            primitiveVertices.data(),
                            static_cast<uint32_t>(primitiveIndices.size()),
                            primitiveIndices.data()
                        );
                    }
                    indicesVertexStartingIndex += primitiveVertexCount;

                }
            }
        }
        return mesh;
    }

	//-------------------------------------------------------------------------------------------------

    std::shared_ptr<MFA::Importer::Model> GLTF_Model(std::string const& path)
    {
        std::shared_ptr<Model> model = nullptr;
        if (MFA_VERIFY(path.empty() == false))
        {
            namespace TG = tinygltf;
            TG::TinyGLTF loader{};
            std::string error;
            std::string warning;
            TG::Model gltfModel{};

            auto const extension = std::filesystem::path(path).extension().string();

            bool success = false;

            if (extension == ".gltf")
            {
                success = loader.LoadASCIIFromFile(
                    &gltfModel,
                    &error,
                    &warning,
                    path
                );
            }
            else if (extension == ".glb")
            {
                success = loader.LoadBinaryFromFile(
                    &gltfModel,
                    &error,
                    &warning,
                    path
                );
            }
            else
            {
                MFA_CRASH("ImportGLTF format is not support: %s", extension.c_str());
            }

            if (error.empty() == false)
            {
                MFA_LOG_ERROR("ImportGltf Error: %s", error.c_str());
            }
            if (warning.empty() == false)
            {
                MFA_LOG_WARN("ImportGltf Warning: %s", warning.c_str());
            }
            if (success)
            {
                std::shared_ptr<Mesh> mesh{};
                std::vector<TextureRef> textureRefs{};

                // TODO Camera
                if (false == gltfModel.meshes.empty())
                {
                    // Textures
                    GLTF_extractTextures(
                        path,
                        gltfModel,
                        textureRefs
                    );

                    // SubMeshes
                    mesh = GLTF_extractSubMeshes(gltfModel, textureRefs);
                    if (mesh == nullptr)
                    {
                        return model;
                    }
                }
                // Nodes
                GLTF_extractNodes(gltfModel, mesh.get());
                // Fill skin
                GLTF_extractSkins(gltfModel, mesh.get());
                // Animation
                GLTF_extractAnimations(gltfModel, mesh.get());
                mesh->FinalizeData();

                std::vector<std::shared_ptr<AS::Texture>> textures{};
                for (size_t i = 0; i < textureRefs.size(); ++i)
                {
                    auto const path = textureRefs[i].relativePath;
                    auto const extension = std::filesystem::path(path).extension().string();

                    std::shared_ptr<AS::Texture> texture{};
                    if (extension == ".png" || extension == ".jpg" || extension == ".jpeg")
                    {
                        texture = Importer::UncompressedImage(path);
                    }
                    else
                    {
                        MFA_ASSERT(false);
                    }

                    MFA_ASSERT(texture != nullptr);
                    textures.emplace_back(texture);
                }

                model = std::make_shared<Model>();
                model->mesh = mesh;
                model->textures = textures;
            }
        }
        return model;
    }

    //-------------------------------------------------------------------------------------------------

}