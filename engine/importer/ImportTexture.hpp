#pragma once

#include <string>

#include "AssetTexture.hpp"

namespace MFA::Importer
{
    struct ImportTextureOptions
    {
        bool tryToGenerateMipmaps = false;      // Generates mipmaps for uncompressed texture
        // TODO Usage flags
    };

    [[nodiscard]]
    std::shared_ptr<AS::Texture> UncompressedImage(
        std::string const& path,
        ImportTextureOptions const& options = {}
    );

    [[nodiscard]]
    std::shared_ptr<AS::Texture> ErrorTexture();

    [[nodiscard]]
    std::shared_ptr<AS::Texture> InMemoryTexture(
        BaseBlob const & data,
        int32_t width,
        int32_t height,
        AS::Texture::Format format,
        uint32_t components,
        uint16_t depth = 1,
        uint16_t slices = 1,
        ImportTextureOptions const& options = {}
    );
}
