#include "ImportTexture.hpp"

#include "BedrockAssert.hpp"
#include "BedrockFile.hpp"
#include "BedrockMemory.hpp"
#include "BedrockPath.hpp"
#include "BedrockPlatforms.hpp"

#include "stb_image.h"
#include "stb_image_resize.h"

namespace MFA::Importer
{

    using Format = AS::Texture::Format;

    struct Data
    {
        int32_t width = 0;
        int32_t height = 0;
        int32_t stbi_components = 0;
        std::shared_ptr<Blob> stbi_pixels;
        Format format = Format::INVALID;
        std::shared_ptr<Blob> pixels;
        uint32_t components = 0;
        [[nodiscard]]
        bool valid() const
        {
            return stbi_pixels != nullptr &&
                stbi_pixels->IsValid() == true &&
                width > 0 &&
                height > 0 &&
                stbi_components > 0;
        }
    };

    enum class LoadResult
    {
        Invalid,
        Success,
        FileNotExists
        // Format not supported
    };

    static LoadResult LoadUncompressed(Data& outImageData, std::string const& path, bool prefer_srgb)
    {
        LoadResult ret = LoadResult::Invalid;

        auto const rawFile = File::Read(path);

        if (rawFile == nullptr)
        {
            return ret;
        }

        auto* readData = stbi_load_from_memory(
            rawFile->Ptr(),
            static_cast<int>(rawFile->Len()),
            &outImageData.width,
            &outImageData.height,
            &outImageData.stbi_components,
            0
        );

        if (readData != nullptr)
        {
            MFA_ASSERT(outImageData.width > 0);
            MFA_ASSERT(outImageData.height > 0);
            MFA_ASSERT(outImageData.stbi_components > 0);

            outImageData.stbi_pixels = std::make_shared<Blob>(
                readData,
                static_cast<size_t>(outImageData.width) *
                outImageData.height *
                outImageData.stbi_components *
                sizeof(uint8_t)

                );

            outImageData.components = outImageData.stbi_components;
            if (prefer_srgb)
            {
                switch (outImageData.stbi_components)
                {
                case 1:
                    outImageData.format = Format::UNCOMPRESSED_UNORM_R8_SRGB;
                    break;
                case 2:
                case 3:
                case 4:
                    outImageData.format = Format::UNCOMPRESSED_UNORM_R8G8B8A8_SRGB;
                    outImageData.components = 4;
                    break;
                default: MFA_NOT_IMPLEMENTED_YET("Mohammad Fakhreddin");
                }
            }
            else
            {
                switch (outImageData.stbi_components)
                {
                case 1:
                    outImageData.format = Format::UNCOMPRESSED_UNORM_R8_LINEAR;
                    break;
                case 2:
                    outImageData.format = Format::UNCOMPRESSED_UNORM_R8G8_LINEAR;
                    break;
                case 3:
                case 4:
                    outImageData.format = Format::UNCOMPRESSED_UNORM_R8G8B8A8_LINEAR;
                    outImageData.components = 4;
                    break;
                default: MFA_LOG_WARN("Unhandled component count: %d", outImageData.stbi_components);
                }
            }
            MFA_ASSERT(outImageData.components >= static_cast<uint32_t>(outImageData.stbi_components));
            if (static_cast<int>(outImageData.components) == outImageData.stbi_components)
            {
                outImageData.pixels = outImageData.stbi_pixels;
            }
            else
            {
                auto const size = static_cast<size_t>(outImageData.width) *
                    outImageData.height *
                    outImageData.components *
                    sizeof(uint8_t);

                outImageData.pixels = Memory::AllocSize(size);

                auto* pixels_array = outImageData.pixels->As<uint8_t>();
                auto const* stbi_pixels_array = outImageData.stbi_pixels->As<uint8_t>();
                for (int pixel_index = 0; pixel_index < outImageData.width * outImageData.height; pixel_index++)
                {
                    for (uint32_t component_index = 0; component_index < outImageData.components; component_index++)
                    {
                        pixels_array[pixel_index * outImageData.components + component_index] = static_cast<int64_t>(component_index) < outImageData.stbi_components
                            ? stbi_pixels_array[pixel_index * outImageData.stbi_components + component_index]
                            : 255u;
                    }
                }
            }
            ret = LoadResult::Success;
        }
        else
        {
            ret = LoadResult::FileNotExists;
        }
        return ret;
    }

    //-------------------------------------------------------------------------------------------------

    struct ResizeInputParams
    {
        BaseBlob inputImagePixels{};
        int32_t inputImageWidth{};
        int32_t inputImageHeight{};
        uint32_t componentsCount{};
        std::shared_ptr<Blob> outputImagePixels{};
        int32_t outputWidth{};
        int32_t outputHeight{};
        bool useSRGB{};
    };

    static bool ResizeUncompressed(ResizeInputParams const& params)
    {
        MFA_ASSERT(params.inputImageWidth > 0);
        MFA_ASSERT(params.inputImageHeight > 0);
        MFA_ASSERT(params.inputImagePixels.Ptr() != nullptr);
        MFA_ASSERT(params.inputImagePixels.Len() > 0);

        MFA_ASSERT(params.componentsCount > 0);

        MFA_ASSERT(params.outputWidth > 0);
        MFA_ASSERT(params.outputHeight > 0);
        MFA_ASSERT(params.outputImagePixels->Ptr() != nullptr);
        MFA_ASSERT(params.outputImagePixels->Len() > 0);

        auto const resizeResult = params.useSRGB ? stbir_resize_uint8_srgb(
            params.inputImagePixels.Ptr(),
            params.inputImageWidth,
            params.inputImageHeight,
            0,
            params.outputImagePixels->Ptr(),
            params.outputWidth,
            params.outputHeight,
            0,
            params.componentsCount,
            params.componentsCount > 3 ? 3 : STBIR_ALPHA_CHANNEL_NONE,
            0
        ) : stbir_resize_uint8(
            params.inputImagePixels.Ptr(),
            params.inputImageWidth,
            params.inputImageHeight,
            0,
            params.outputImagePixels->Ptr(),
            params.outputWidth,
            params.outputHeight,
            0,
            params.componentsCount
        );
        return resizeResult > 0;
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<AS::Texture> UncompressedImage(
        std::string const& path, 
        ImportTextureOptions const& options
    )
    {
        std::shared_ptr<AS::Texture> texture{};
        Data imageData{};
        auto const loadImageResult = LoadUncompressed(
            imageData,
            path,
            false
        );
        if (loadImageResult == LoadResult::Success)
        {
            MFA_ASSERT(imageData.valid());
            auto const image_width = imageData.width;
            auto const image_height = imageData.height;
            auto const pixels = imageData.pixels;
            auto const components = imageData.components;
            auto const format = imageData.format;
            int const depth = 1; // TODO We need to support depth
            int const slices = 1;

            texture = InMemoryTexture(
                *pixels,
                image_width,
                image_height,
                format,
                components,
                depth,
                slices,
                options
            );
        }
        // TODO: Handle errors
        return texture;
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<AS::Texture> ErrorTexture()
    {
        auto const data = Memory::AllocSize(4);
        auto* pixel = data->As<uint8_t>();
        pixel[0] = 1;
        pixel[1] = 1;
        pixel[2] = 1;
        pixel[3] = 1;

        return InMemoryTexture(
            *data,
            1,
            1,
            AS::Texture::Format::UNCOMPRESSED_UNORM_R8G8B8A8_LINEAR,
            4,
            1,
            1
        );
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<AS::Texture> InMemoryTexture(
        BaseBlob const & data,
        int32_t width,
        int32_t height,
        Format format,
        uint32_t components,
        uint16_t depth,
        uint16_t slices,
        ImportTextureOptions const& options
    )
    {
        AS::Texture::Dimensions const originalImageDimension{
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height),
            depth
        };

        uint8_t const mipCount = options.tryToGenerateMipmaps
            ? AS::Texture::ComputeMipCount(originalImageDimension)
            : 1;

        auto const bufferSize = AS::Texture::CalculateUncompressedTextureRequiredDataSize(
            format,
            slices,
            originalImageDimension,
            mipCount
        );

        std::shared_ptr<AS::Texture> texture = std::make_shared<AS::Texture>(
            format,
            slices,
            depth,
            bufferSize
        );

        // Generating mipmaps (TODO : Code needs debugging)
        texture->addMipmap(originalImageDimension, std::make_shared<Blob>(data));

        for (uint8_t mipLevel = 1; mipLevel < mipCount; mipLevel++)
        {
            auto const currentMipDims = AS::Texture::MipDimensions(
                mipLevel,
                mipCount,
                originalImageDimension
            );
            auto const currentMipSizeBytes = AS::Texture::MipSizeBytes(
                format,
                slices,
                currentMipDims
            );
            std::shared_ptr<Blob> mipMapPixels = Memory::AllocSize(currentMipSizeBytes);

            // Resize
            ResizeInputParams inputParams{
                .inputImagePixels = data,
                .inputImageWidth = static_cast<int>(originalImageDimension.width),
                .inputImageHeight = static_cast<int>(originalImageDimension.height),
                .componentsCount = components,
                .outputImagePixels = mipMapPixels,
                .outputWidth = static_cast<int>(currentMipDims.width),
                .outputHeight = static_cast<int>(currentMipDims.height),
            };
            auto const resizeResult = ResizeUncompressed(inputParams);
            MFA_ASSERT(resizeResult == true);

            texture->addMipmap(
                currentMipDims,
                mipMapPixels
            );
        }

        MFA_ASSERT(texture->isValid());

        return texture;
    }
}
