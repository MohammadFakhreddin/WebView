#pragma once

#include "pipeline/TextOverlayPipeline.hpp"
#include "BufferTracker.hpp"

#include "stb_truetype.h"

#include <optional>

namespace MFA
{
    class CustomFontRenderer
    {
    public:

        using Pipeline = TextOverlayPipeline;

        static constexpr float DefaultFontSize = 14.0f;

        struct TextData
        {
            std::vector<int> letterRange{};
            std::optional<LocalBufferTracker> vertexData = std::nullopt;
            const int maxLetterCount;
        };
        // TODO: We have to pass the command buffer here.
        explicit CustomFontRenderer(
            std::shared_ptr<Pipeline> pipeline,
            Alias const & fontData,
            float fontHeight
        );

        std::unique_ptr<TextData> AllocateTextData(int maxCharCount = 2048);

        enum class HorizontalTextAlign {Center, Left, Right};

        struct TextParams
        {
            HorizontalTextAlign hTextAlign = HorizontalTextAlign::Left;
            float fontSizeInPixels = DefaultFontSize;
            glm::vec3 color{1.0f, 1.0f, 1.0f};
        };

        bool AddText(
            TextData & inOutData,
            std::string_view const & text,
            float x,
            float y,
            TextParams const & params
        );

        void ResetText(TextData & inOutData);

        void Draw(
            RT::CommandRecordState& recordState,
            Pipeline::PushConstants const & pushConstants,
            TextData & data
        ) const;

        [[nodiscard]]
        float TextWidth(std::string_view const& text, float fontSizeInPixels = DefaultFontSize) const;

        [[nodiscard]]
        float TextHeight(float fontSizeInPixels = DefaultFontSize) const;

    private:

        void CreateFontTextureBuffer(uint32_t width, uint32_t height, Alias const & bytes);

    public:

        static constexpr float WidthModifier = 1.0f;
        static constexpr float HeightModifier = 1.0f;
        static constexpr int FontFirstChar = 32;
        static constexpr int FontNumberOfChars = 224;

    private:

        std::vector<stbtt_bakedchar> _stbFontData{};

        std::shared_ptr<RT::GpuTexture> _fontTexture{};

        std::shared_ptr<RT::SamplerGroup> _fontSampler{};

        std::shared_ptr<Pipeline> _pipeline{};

        RT::DescriptorSetGroup _descriptorSet{};

        float _atlasWidth{};
        float _atlasHeight{};
        float _fontHeight{};
    };
}
