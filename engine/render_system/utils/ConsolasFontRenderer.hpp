#pragma once

#include "pipeline/TextOverlayPipeline.hpp"
#include "BufferTracker.hpp"

#include "stb_font_consolas_24_latin1.h"

#include <optional>

namespace MFA
{

    class ConsolasFontRenderer
    {
    public:

        struct TextData
        {
            std::vector<int> letterRange{};
            std::optional<LocalBufferTracker> vertexData = std::nullopt;
            const int maxLetterCount;
        };

        explicit ConsolasFontRenderer(std::shared_ptr<TextOverlayPipeline> pipeline);

        std::unique_ptr<TextData> AllocateTextData(int maxCharCount = 2048);

        enum class HorizontalTextAlign {Center, Left, Right};

        enum class VerticalTextAlign { Top, Center };

        struct TextParams
        {
            HorizontalTextAlign hTextAlign = HorizontalTextAlign::Left;
            VerticalTextAlign vTextAlign = VerticalTextAlign::Top;
            float scale = 1.5f;
            glm::vec3 color{1.0f, 1.0f, 1.0f};
        };
        
        bool AddText(
            TextData & inOutData,
            std::string_view const & text, 
            float x, 
            float y, 
            TextParams params
        );

        void ResetText(TextData & inOutData);

        void Draw(
            RT::CommandRecordState& recordState,
            TextData & data
        ) const;

        [[nodiscard]]
        float TextWidth(std::string_view const& text, TextParams params);

        [[nodiscard]]
        float TextHeight(float textScale = 1.5f) const;

    private:

        void CreateFontTextureBuffer();

        stb_fontchar _stbFontData[STB_FONT_consolas_24_latin1_NUM_CHARS]{};

        std::shared_ptr<RT::GpuTexture> _fontTexture{};
        
        std::shared_ptr<RT::SamplerGroup> _fontSampler{};

        std::shared_ptr<TextOverlayPipeline> _pipeline{};

        RT::DescriptorSetGroup _descriptorSet{};

        float _fontHeight = 0.0f;

        static constexpr float WidthModifier = 1.5f;
        static constexpr float HeightModifier = 2.0f;
  
    };

}