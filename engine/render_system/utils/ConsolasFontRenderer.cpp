#include "ConsolasFontRenderer.hpp"

#include "LogicalDevice.hpp"

namespace MFA
{

    //------------------------------------------------------------------

    ConsolasFontRenderer::ConsolasFontRenderer(std::shared_ptr<TextOverlayPipeline> pipeline)
        : _pipeline(std::move(pipeline))
    {
        CreateFontTextureBuffer();
        _descriptorSet = _pipeline->CreateDescriptorSet(*_fontTexture);
    }
        
    //------------------------------------------------------------------

    std::unique_ptr<ConsolasFontRenderer::TextData> ConsolasFontRenderer::AllocateTextData(int maxCharCount)
    {
        auto const device = LogicalDevice::Instance;

        auto const vertexBuffer = RB::CreateVertexBufferGroup(
            device->GetVkDevice(), 
            device->GetPhysicalDevice(), 
            maxCharCount * sizeof(TextOverlayPipeline::Vertex),
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

    // TODO: Add support for color
    bool ConsolasFontRenderer::AddText(
        TextData & inOutData,
        std::string_view const & text, 
        float x, 
        float y, 
        AddTextParams params
    )
    {
        auto const windowWidth = static_cast<float>(LogicalDevice::Instance->GetWindowWidth());
        auto const windowHeight = static_cast<float>(LogicalDevice::Instance->GetWindowHeight());

        const uint32_t firstChar = STB_FONT_consolas_24_latin1_FIRST_CHAR;

        int letterRange = inOutData.letterRange.empty() == false ? inOutData.letterRange.back() : 0;
        
        auto * mapped = &reinterpret_cast<TextOverlayPipeline::Vertex*>(inOutData.vertexData->Data())[letterRange * 4];
        
        const float charW = 1.5f * params.scale / windowWidth;
        const float charH = 1.5f * params.scale / windowHeight;

        float fbW = windowWidth;
        float fbH = windowHeight;
        x = (x / fbW * 2.0f) - 1.0f;
        y = (y / fbH * 2.0f) - 1.0f;

        // Calculate text width
        float textWidth = 0;
        for (auto letter : text)
        {
            stb_fontchar *charData = &_stbFontData[(uint32_t)letter - firstChar];
            textWidth += charData->advance * charW;
        }

        switch (params.textAlign)
        {
            case TextAlign::Right:
                x -= textWidth;
                break;
            case TextAlign::Center:
                x -= textWidth / 2.0f;
                break;
            case TextAlign::Left:
                break;
        }

        bool success = true;

        // Generate a uv mapped quad per char in the new text
        for (auto letter : text)
        {
            if (letterRange + 1 >= inOutData.maxLetterCount)
            {
                success = false;
                break;
            }

            stb_fontchar *charData = &_stbFontData[(uint32_t)letter - firstChar];

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
