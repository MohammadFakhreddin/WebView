#include "ConsolasFontRenderer.hpp"

#include "LogicalDevice.hpp"

namespace MFA
{

    //------------------------------------------------------------------

    static constexpr uint32_t firstChar = STB_FONT_consolas_24_latin1_FIRST_CHAR;

    //------------------------------------------------------------------

    ConsolasFontRenderer::ConsolasFontRenderer(std::shared_ptr<Pipeline> pipeline)
        : _pipeline(std::move(pipeline))
    {
        CreateFontTextureBuffer();
        _descriptorSet = _pipeline->CreateDescriptorSet(*_fontTexture);

        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::min();
        for (int i = 0; i < STB_FONT_consolas_24_latin1_NUM_CHARS; ++i)
        {
            stb_fontchar * charData = &_stbFontData[i];
            minY = std::min<float>(charData->y0, minY);
            minY = std::min<float>(charData->y1, minY);

            maxY = std::max<float>(charData->y0, maxY);
            maxY = std::max<float>(charData->y1, maxY);
        }

        _fontHeight = std::abs(maxY - minY);
    }

    //------------------------------------------------------------------

    std::unique_ptr<ConsolasFontRenderer::TextData> ConsolasFontRenderer::AllocateTextData(int maxCharCount)
    {
        auto const device = LogicalDevice::Instance;

        auto const vertexBuffer = RB::CreateVertexBufferGroup(
            device->GetVkDevice(),
            device->GetPhysicalDevice(),
            maxCharCount * sizeof(Pipeline::Vertex),
            LogicalDevice::Instance->GetMaxFramePerFlight()
        );

        auto const vertexStageBuffer = RB::CreateStageBuffer(
            device->GetVkDevice(),
            device->GetPhysicalDevice(),
            vertexBuffer->bufferSize,
            vertexBuffer->buffers.size()
        );

        std::unique_ptr<TextData> textData = std::make_unique<TextData>(TextData{
            .letterRange = {},
            .vertexData = LocalBufferTracker(vertexBuffer, vertexStageBuffer),
            .maxLetterCount = maxCharCount
        });

        return textData;
    }

    //------------------------------------------------------------------

    bool ConsolasFontRenderer::AddText(
        TextData & inOutData,
        std::string_view const & text,
        float x,
        float y,
        TextParams params
    )
    {
        int letterRange = inOutData.letterRange.empty() == false ? inOutData.letterRange.back() : 0;
        int letterRangeBegin = letterRange;

        auto * mapped = &reinterpret_cast<Pipeline::Vertex*>(inOutData.vertexData->Data())[letterRange * 4];
        auto* mappedBegin = mapped;

        const float charW = WidthModifier * params.scale;
        const float charH = HeightModifier * params.scale;

        bool success = true;

        // Generate a uv mapped quad per char in the new text
        for (auto letter : text)
        {
            if (letterRange + 1 >= inOutData.maxLetterCount)
            {
                success = false;
                break;
            }

            int letterIdx = static_cast<uint32_t>(letter) - firstChar;
            if (letterIdx >= 0 && letterIdx < STB_FONT_consolas_24_latin1_NUM_CHARS)
            {
                stb_fontchar* charData = &_stbFontData[letterIdx];

                mapped->position.x = (x + (float)charData->x0 * charW);
                mapped->position.y = (y + (float)charData->y0 * charH);
                mapped->uv.x = charData->s0;
                mapped->uv.y = charData->t0;
                mapped->color = params.color;
                
                mapped++;

                mapped->position.x = (x + (float)charData->x1 * charW);
                mapped->position.y = (y + (float)charData->y0 * charH);
                mapped->uv.x = charData->s1;
                mapped->uv.y = charData->t0;
                mapped->color = params.color;
                
                mapped++;

                mapped->position.x = (x + (float)charData->x0 * charW);
                mapped->position.y = (y + (float)charData->y1 * charH);
                mapped->uv.x = charData->s0;
                mapped->uv.y = charData->t1;
                mapped->color = params.color;
                
                mapped++;

                mapped->position.x = (x + (float)charData->x1 * charW);
                mapped->position.y = (y + (float)charData->y1 * charH);
                mapped->uv.x = charData->s1;
                mapped->uv.y = charData->t1;
                mapped->color = params.color;
                
                mapped++;

                x += charData->advance * charW;

                letterRange++;
            }
        }

        mapped = mappedBegin;
        int itrCount = std::min<int>(inOutData.maxLetterCount - letterRangeBegin, text.size() * 4);

        switch (params.vTextAlign)
        {
        case VerticalTextAlign::Center:
            for (int i = 0; i < itrCount; ++i)
            {
                mapped[i].position.y -= _fontHeight * charH;
            }
            break;
        }
    	
        auto const width = TextWidth(text, params);
        switch (params.hTextAlign)
        {
        case HorizontalTextAlign::Right:
            for (int i = 0; i < itrCount; ++i)
            {
                mappedBegin[i].position.x -= width;
            }
            break;
        case HorizontalTextAlign::Center:
            auto const halfWidth = width * 0.5f;
            for (int i = 0; i < itrCount; ++i)
            {
                mappedBegin[i].position.x -= halfWidth;
            }
            break;
        }

        inOutData.letterRange.emplace_back(letterRange);

        return success;
    }

    //------------------------------------------------------------------

    void ConsolasFontRenderer::ResetText(TextData & inOutData)
    {
        inOutData.letterRange.clear();
    }

    //------------------------------------------------------------------

    void ConsolasFontRenderer::Draw(
        RT::CommandRecordState& recordState,
        Pipeline::PushConstants const & pushConstants,
        TextData& data
    ) const
    {
        _pipeline->BindPipeline(recordState);

        RB::AutoBindDescriptorSet(
            recordState,
            RB::UpdateFrequency::PerPipeline,
            _descriptorSet.descriptorSets[0]
        );

        RB::BindVertexBuffer(recordState, *data.vertexData->LocalBuffer().buffers[recordState.frameIndex]);

        int previousLetterRange = 0;
        for (auto const& letterRange : data.letterRange)
        {
            int const currentLetterRange = letterRange * 4;
            vkCmdDraw(recordState.commandBuffer, currentLetterRange - previousLetterRange, 1, previousLetterRange, 0);
            previousLetterRange = currentLetterRange;
        }
    }

    //------------------------------------------------------------------

    float ConsolasFontRenderer::TextWidth(std::string_view const& text, TextParams params)
    {
        const float charW = WidthModifier * params.scale;

        float textWidth = 0;
        for (auto letter : text)
        {
            int idx = static_cast<uint32_t>(letter) - firstChar;
            if (idx >= 0 && idx < STB_FONT_consolas_24_latin1_NUM_CHARS)
            {
                stb_fontchar* charData = &_stbFontData[static_cast<uint32_t>(letter) - firstChar];
                textWidth += charData->advance * charW;
            }
        }

        return textWidth;
    }

    //------------------------------------------------------------------

    float ConsolasFontRenderer::TextHeight(float const textScale) const
    {
        const float charH = HeightModifier * textScale;
        return charH * _fontHeight;
    }

    //------------------------------------------------------------------

    void ConsolasFontRenderer::CreateFontTextureBuffer()
    {
        auto const device = LogicalDevice::Instance;

        auto const fontWidth = STB_FONT_consolas_24_latin1_BITMAP_WIDTH;
        auto const fontHeight = STB_FONT_consolas_24_latin1_BITMAP_HEIGHT;

        uint8_t font24pixels[fontHeight][fontWidth];
        stb_font_consolas_24_latin1(_stbFontData, font24pixels, fontHeight);

        AS::Texture cpuTexture {
            Asset::Texture::Format::UNCOMPRESSED_UNORM_R8_LINEAR,
            1,
            1,
            sizeof(font24pixels)
        };
        
        cpuTexture.addMipmap(
            Asset::Texture::Dimensions {
                .width = fontWidth,
                .height = fontHeight,
                .depth = 1
            },
            &font24pixels[0][0], 
            sizeof(font24pixels)
        );

        auto commandBuffer = RB::BeginSingleTimeCommand(
            device->GetVkDevice(), 
            device->GetGraphicCommandPool()
        );
        
        auto [fontTexture, stageBuffer] = RB::CreateTexture(
            cpuTexture,         
            device->GetVkDevice(),
            device->GetPhysicalDevice(),
            commandBuffer
        );
        MFA_ASSERT(fontTexture != nullptr);
        _fontTexture = std::move(fontTexture);
        
        RB::EndAndSubmitSingleTimeCommand(
            device->GetVkDevice(), 
            device->GetGraphicCommandPool(),
            device->GetGraphicQueue(),
            commandBuffer
        );
    }
    
    //------------------------------------------------------------------

}
