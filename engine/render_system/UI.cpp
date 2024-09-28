#include "UI.hpp"

#include "BedrockPlatforms.hpp"

#include <cstdint>

#include "imgui.h"
#include "LogicalDevice.hpp"
#include "ImportShader.hpp"
#include "ImportTexture.hpp"

namespace MFA
{

    //-----------------------------------------------------------------------------
	// SHADERS
	//-----------------------------------------------------------------------------

	// glsl_shader.vert, compiled with:
	// # glslangValidator -V -x -o glsl_shader.vert.u32 glsl_shader.vert
	/*
	#version 450 core
	layout(location = 0) in vec2 aPos;d
	layout(location = 1) in vec2 aUV;
	layout(location = 2) in vec4 aColor;
	layout(push_constant) uniform uPushConstant { vec2 uScale; vec2 uTranslate; } pc;

	out gl_PerVertex { vec4 gl_Position; };
	layout(location = 0) out struct { vec4 Color; vec2 UV; } Out;

	void main()
	{
	    Out.Color = aColor;
	    Out.UV = aUV;
	    gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
	}
	*/
    static uint32_t vertex_shader_spv[] =
    {
        0x07230203,0x00010000,0x00080001,0x0000002e,0x00000000,0x00020011,0x00000001,0x0006000b,
        0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
        0x000a000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000b,0x0000000f,0x00000015,
        0x0000001b,0x0000001c,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
        0x00000000,0x00030005,0x00000009,0x00000000,0x00050006,0x00000009,0x00000000,0x6f6c6f43,
        0x00000072,0x00040006,0x00000009,0x00000001,0x00005655,0x00030005,0x0000000b,0x0074754f,
        0x00040005,0x0000000f,0x6c6f4361,0x0000726f,0x00030005,0x00000015,0x00565561,0x00060005,
        0x00000019,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x00000019,0x00000000,
        0x505f6c67,0x7469736f,0x006e6f69,0x00030005,0x0000001b,0x00000000,0x00040005,0x0000001c,
        0x736f5061,0x00000000,0x00060005,0x0000001e,0x73755075,0x6e6f4368,0x6e617473,0x00000074,
        0x00050006,0x0000001e,0x00000000,0x61635375,0x0000656c,0x00060006,0x0000001e,0x00000001,
        0x61725475,0x616c736e,0x00006574,0x00030005,0x00000020,0x00006370,0x00040047,0x0000000b,
        0x0000001e,0x00000000,0x00040047,0x0000000f,0x0000001e,0x00000002,0x00040047,0x00000015,
        0x0000001e,0x00000001,0x00050048,0x00000019,0x00000000,0x0000000b,0x00000000,0x00030047,
        0x00000019,0x00000002,0x00040047,0x0000001c,0x0000001e,0x00000000,0x00050048,0x0000001e,
        0x00000000,0x00000023,0x00000000,0x00050048,0x0000001e,0x00000001,0x00000023,0x00000008,
        0x00030047,0x0000001e,0x00000002,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,
        0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040017,
        0x00000008,0x00000006,0x00000002,0x0004001e,0x00000009,0x00000007,0x00000008,0x00040020,
        0x0000000a,0x00000003,0x00000009,0x0004003b,0x0000000a,0x0000000b,0x00000003,0x00040015,
        0x0000000c,0x00000020,0x00000001,0x0004002b,0x0000000c,0x0000000d,0x00000000,0x00040020,
        0x0000000e,0x00000001,0x00000007,0x0004003b,0x0000000e,0x0000000f,0x00000001,0x00040020,
        0x00000011,0x00000003,0x00000007,0x0004002b,0x0000000c,0x00000013,0x00000001,0x00040020,
        0x00000014,0x00000001,0x00000008,0x0004003b,0x00000014,0x00000015,0x00000001,0x00040020,
        0x00000017,0x00000003,0x00000008,0x0003001e,0x00000019,0x00000007,0x00040020,0x0000001a,
        0x00000003,0x00000019,0x0004003b,0x0000001a,0x0000001b,0x00000003,0x0004003b,0x00000014,
        0x0000001c,0x00000001,0x0004001e,0x0000001e,0x00000008,0x00000008,0x00040020,0x0000001f,
        0x00000009,0x0000001e,0x0004003b,0x0000001f,0x00000020,0x00000009,0x00040020,0x00000021,
        0x00000009,0x00000008,0x0004002b,0x00000006,0x00000028,0x00000000,0x0004002b,0x00000006,
        0x00000029,0x3f800000,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,
        0x00000005,0x0004003d,0x00000007,0x00000010,0x0000000f,0x00050041,0x00000011,0x00000012,
        0x0000000b,0x0000000d,0x0003003e,0x00000012,0x00000010,0x0004003d,0x00000008,0x00000016,
        0x00000015,0x00050041,0x00000017,0x00000018,0x0000000b,0x00000013,0x0003003e,0x00000018,
        0x00000016,0x0004003d,0x00000008,0x0000001d,0x0000001c,0x00050041,0x00000021,0x00000022,
        0x00000020,0x0000000d,0x0004003d,0x00000008,0x00000023,0x00000022,0x00050085,0x00000008,
        0x00000024,0x0000001d,0x00000023,0x00050041,0x00000021,0x00000025,0x00000020,0x00000013,
        0x0004003d,0x00000008,0x00000026,0x00000025,0x00050081,0x00000008,0x00000027,0x00000024,
        0x00000026,0x00050051,0x00000006,0x0000002a,0x00000027,0x00000000,0x00050051,0x00000006,
        0x0000002b,0x00000027,0x00000001,0x00070050,0x00000007,0x0000002c,0x0000002a,0x0000002b,
        0x00000028,0x00000029,0x00050041,0x00000011,0x0000002d,0x0000001b,0x0000000d,0x0003003e,
        0x0000002d,0x0000002c,0x000100fd,0x00010038
    };

    // glsl_shader.frag, compiled with:
    // # glslangValidator -V -x -o glsl_shader.frag.u32 glsl_shader.frag
    /*
    #version 450 core
    layout(location = 0) out vec4 fColor;
    layout(set=0, binding=0) uniform sampler2D sTexture;
    layout(location = 0) in struct { vec4 Color; vec2 UV; } In;
    void main()
    {
        fColor = In.Color * texture(sTexture, In.UV.st);
    }
    */
    static uint32_t fragment_shader_spv[] =
    {
        0x07230203,0x00010000,0x00080001,0x0000001e,0x00000000,0x00020011,0x00000001,0x0006000b,
        0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
        0x0007000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000d,0x00030010,
        0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
        0x00000000,0x00040005,0x00000009,0x6c6f4366,0x0000726f,0x00030005,0x0000000b,0x00000000,
        0x00050006,0x0000000b,0x00000000,0x6f6c6f43,0x00000072,0x00040006,0x0000000b,0x00000001,
        0x00005655,0x00030005,0x0000000d,0x00006e49,0x00050005,0x00000016,0x78655473,0x65727574,
        0x00000000,0x00040047,0x00000009,0x0000001e,0x00000000,0x00040047,0x0000000d,0x0000001e,
        0x00000000,0x00040047,0x00000016,0x00000022,0x00000000,0x00040047,0x00000016,0x00000021,
        0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,
        0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040020,0x00000008,0x00000003,
        0x00000007,0x0004003b,0x00000008,0x00000009,0x00000003,0x00040017,0x0000000a,0x00000006,
        0x00000002,0x0004001e,0x0000000b,0x00000007,0x0000000a,0x00040020,0x0000000c,0x00000001,
        0x0000000b,0x0004003b,0x0000000c,0x0000000d,0x00000001,0x00040015,0x0000000e,0x00000020,
        0x00000001,0x0004002b,0x0000000e,0x0000000f,0x00000000,0x00040020,0x00000010,0x00000001,
        0x00000007,0x00090019,0x00000013,0x00000006,0x00000001,0x00000000,0x00000000,0x00000000,
        0x00000001,0x00000000,0x0003001b,0x00000014,0x00000013,0x00040020,0x00000015,0x00000000,
        0x00000014,0x0004003b,0x00000015,0x00000016,0x00000000,0x0004002b,0x0000000e,0x00000018,
        0x00000001,0x00040020,0x00000019,0x00000001,0x0000000a,0x00050036,0x00000002,0x00000004,
        0x00000000,0x00000003,0x000200f8,0x00000005,0x00050041,0x00000010,0x00000011,0x0000000d,
        0x0000000f,0x0004003d,0x00000007,0x00000012,0x00000011,0x0004003d,0x00000014,0x00000017,
        0x00000016,0x00050041,0x00000019,0x0000001a,0x0000000d,0x00000018,0x0004003d,0x0000000a,
        0x0000001b,0x0000001a,0x00050057,0x00000007,0x0000001c,0x00000017,0x0000001b,0x00050085,
        0x00000007,0x0000001d,0x00000012,0x0000001c,0x0003003e,0x00000009,0x0000001d,0x000100fd,
        0x00010038
    };

    //-------------------------------------------------------------------------------------------------

    static int EventWatch(SDL_Event* event)
    {
        ImGuiIO& io = ImGui::GetIO();
        switch (event->type)
        {
        case SDL_TEXTINPUT:
        {
            io.AddInputCharactersUTF8(event->text.text);
            return true;
        }
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            const int key = event->key.keysym.scancode;
            IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown));
            io.KeysDown[key] = (event->type == SDL_KEYDOWN);
            io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
            io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
            io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
#ifdef __PLATFORM_WIN__
            io.KeySuper = false;
#else
            io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
#endif
            return true;
        }
        }
        return false;
    }

	//-------------------------------------------------------------------------------------------------

	UI::UI(std::shared_ptr<DisplayRenderPass> displayRenderPass)
    {
        _displayRenderPass = std::move(displayRenderPass);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        
        // FontSampler
        _fontSampler = RB::CreateSampler(
            LogicalDevice::Instance->GetVkDevice(),
            RB::CreateSamplerParams{
                .maxAnisotropy = 1.0f,
                .minLod = -1000,
                .maxLod = 1000,
            }
        );

        _descriptorPool = RB::CreateDescriptorPool(LogicalDevice::Instance->GetVkDevice(), LogicalDevice::Instance->GetMaxFramePerFlight());

        CreateDescriptorSetLayout();

        CreatePipeline();

        CreateFontTexture();

        BindKeyboard();

        ImGui::GetIO().FontGlobalScale = 1.0f;

        UpdateDescriptorSets();

        _eventWatchId = LogicalDevice::Instance->SDL_EventSignal.Register([this](SDL_Event * event)->void {EventWatch(event); });

        //OnResize();

        _resizeSignalId = LogicalDevice::Instance->ResizeEventSignal2.Register([this]()->void {OnResize(); });

        auto const maxFramesPerFlight = LogicalDevice::Instance->GetMaxFramePerFlight();
        _vertexBuffers.resize(maxFramesPerFlight);
        _indexBuffers.resize(maxFramesPerFlight);

        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

        MFA_ASSERT(Instance == nullptr);
        Instance = this;
    }

	//-------------------------------------------------------------------------------------------------

	UI::~UI()
    {
        LogicalDevice::Instance->ResizeEventSignal2.UnRegister(_resizeSignalId);
        LogicalDevice::Instance->SDL_EventSignal.UnRegister(_eventWatchId);

        MFA_ASSERT(Instance != nullptr);
        Instance = nullptr;

    }

    //-------------------------------------------------------------------------------------------------

	void UI::Update()
	{
        ImGui::NewFrame();
        _hasFocus = false;
        UpdateSignal.Emit();
        ImGui::Render();
	}

    //-------------------------------------------------------------------------------------------------

	bool UI::Render(
        RT::CommandRecordState& recordState,
        float const deltaTimeInSec
    )
	{
        ImGuiIO& io = ImGui::GetIO();
        MFA_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer backend. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

        io.DeltaTime = deltaTimeInSec;
        UpdateMousePositionAndButtons();
        UpdateMouseCursor();

        auto const* drawData = ImGui::GetDrawData();
        if (drawData == nullptr)
        {
            return false;
        }

        // Setup desired Vulkan state
        // Bind pipeline and descriptor sets:
        RB::BindPipeline(recordState, *_pipeline);

        RB::AutoBindDescriptorSet(
            recordState,
            RB::UpdateFrequency::PerPipeline,
            _descriptorSetGroup.descriptorSets[0]
        );

        // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
        float const frameBufferWidth = drawData->DisplaySize.x * drawData->FramebufferScale.x;
        float const frameBufferHeight = drawData->DisplaySize.y * drawData->FramebufferScale.y;
        if (frameBufferWidth > 0 && frameBufferHeight > 0)
        {
            if (drawData->TotalVtxCount > 0)
            {
                // TODO We can create vertices for ui system in post render
                // Create or resize the vertex/index buffers
                size_t const vertexSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
                size_t const indexSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);
                auto const vertexData = Memory::AllocSize(vertexSize);
                auto const indexData = Memory::AllocSize(indexSize);
                {
                    auto* vertexPtr = reinterpret_cast<ImDrawVert*>(vertexData->Ptr());
                    auto* indexPtr = reinterpret_cast<ImDrawIdx*>(indexData->Ptr());
                    for (int n = 0; n < drawData->CmdListsCount; n++)
                    {
                        const ImDrawList* cmd = drawData->CmdLists[n];
                        ::memcpy(vertexPtr, cmd->VtxBuffer.Data, cmd->VtxBuffer.Size * sizeof(ImDrawVert));
                        ::memcpy(indexPtr, cmd->IdxBuffer.Data, cmd->IdxBuffer.Size * sizeof(ImDrawIdx));
                        vertexPtr += cmd->VtxBuffer.Size;
                        indexPtr += cmd->IdxBuffer.Size;
                    }
                }

                auto& vertexBuffer = _vertexBuffers[recordState.frameIndex];
                auto& indexBuffer = _indexBuffers[recordState.frameIndex];

                auto device = LogicalDevice::Instance->GetVkDevice();
                auto physicalDevice = LogicalDevice::Instance->GetPhysicalDevice();

                if (vertexBuffer == nullptr || vertexBuffer->size < vertexSize)
                {
                    vertexBuffer = RB::CreateBuffer(
                        device,
                        physicalDevice,
                        vertexSize,
                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                    );
                }

                RB::UpdateHostVisibleBuffer(
                    device,
                    *vertexBuffer,
                    *vertexData
                );

                if (indexBuffer == nullptr || indexBuffer->size < indexSize)
                {
                    indexBuffer = RB::CreateBuffer(
                        device,
                        physicalDevice,
                        indexSize,
                        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                    );
                }

                RB::UpdateHostVisibleBuffer(
                    device,
                    *indexBuffer, 
                    *indexData
                );

                RB::BindIndexBuffer(
                    recordState,
                    *indexBuffer,
                    0,
                    sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32
                );

                RB::BindVertexBuffer(
                    recordState,
                    *vertexBuffer,
                    0,
                    0
                );

                // Setup viewport:
                VkViewport const viewport
                {
                    .x = 0,
                    .y = 0,
                    .width = frameBufferWidth,
                    .height = frameBufferHeight,
                    .minDepth = 0.0f,
                    .maxDepth = 1.0f,
                };
                RB::SetViewport(recordState.commandBuffer, viewport);

                // Setup scale and translation:
                // Our visible imgui space lies from draw_data->DisplayPps (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
                {
                    PushConstants constants{};
                    constants.scale[0] = 2.0f / drawData->DisplaySize.x;
                    constants.scale[1] = 2.0f / drawData->DisplaySize.y;
                    constants.translate[0] = -1.0f - drawData->DisplayPos.x * constants.scale[0];
                    constants.translate[1] = -1.0f - drawData->DisplayPos.y * constants.scale[1];
                    RB::PushConstants(
                        recordState,
                        _pipeline->pipelineLayout,
                        VK_SHADER_STAGE_VERTEX_BIT,
                        0,
                        Alias(constants)
                    );
                }

                // Will project scissor/clipping rectangles into frame-buffer space
                ImVec2 const clip_off = drawData->DisplayPos;         // (0,0) unless using multi-viewports
                ImVec2 const clip_scale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

                // Render command lists
                // (Because we merged all buffers into a single one, we maintain our own offset into them)
                int global_vtx_offset = 0;
                int global_idx_offset = 0;
                for (int n = 0; n < drawData->CmdListsCount; n++)
                {
                    const ImDrawList* cmd_list = drawData->CmdLists[n];
                    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
                    {
                        const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

                        // Project scissor/clipping rectangles into frame-buffer space
                        ImVec4 clip_rect;
                        clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
                        clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
                        clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
                        clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

                        if (clip_rect.x < frameBufferWidth && clip_rect.y < frameBufferHeight && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                        {
                            // Negative offsets are illegal for vkCmdSetScissor
                            if (clip_rect.x < 0.0f)
                                clip_rect.x = 0.0f;
                            if (clip_rect.y < 0.0f)
                                clip_rect.y = 0.0f;

                            {// Apply scissor/clipping rectangle
                                VkRect2D scissor{};
                                scissor.offset.x = static_cast<int32_t>(clip_rect.x);
                                scissor.offset.y = static_cast<int32_t>(clip_rect.y);
                                scissor.extent.width = static_cast<uint32_t>(clip_rect.z - clip_rect.x);
                                scissor.extent.height = static_cast<uint32_t>(clip_rect.w - clip_rect.y);
                                RB::SetScissor(recordState.commandBuffer, scissor);
                            }

                            // Draw
                            RB::DrawIndexed(
                                recordState,
                                pcmd->ElemCount,
                                1,
                                pcmd->IdxOffset + global_idx_offset,
                                pcmd->VtxOffset + global_vtx_offset
                            );
                        }
                    }
                    global_idx_offset += cmd_list->IdxBuffer.Size;
                    global_vtx_offset += cmd_list->VtxBuffer.Size;
                }
            }
        }

        return true;
	}

	//-------------------------------------------------------------------------------------------------

	bool UI::HasFocus() const
	{
        return _hasFocus;
	}

    //-------------------------------------------------------------------------------------------------

	void UI::BeginWindow(std::string const& windowName)
	{
        ImGui::Begin(windowName.c_str());
	}

    //-------------------------------------------------------------------------------------------------

	void UI::EndWindow()
	{
        if (ImGui::IsWindowFocused() || ImGui::IsWindowHovered())
        {
            _hasFocus = true;
        }
        ImGui::End();
	}

	//-------------------------------------------------------------------------------------------------

	void UI::OnResize()
	{
        ImGuiIO& io = ImGui::GetIO();
        MFA_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer backend. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

        // Setup display size (every frame to accommodate for window resizing)
        auto const extent = LogicalDevice::Instance->GetSurfaceCapabilities().currentExtent;
        auto windowWidth = extent.width;
        auto windowHeight = extent.height;

        if (LogicalDevice::Instance->IsWindowVisible() == false)
        {
            windowWidth = 0.0f;
            windowHeight = 0.0f;
        }
	    
        io.DisplaySize = ImVec2(static_cast<float>(windowWidth), static_cast<float>(windowHeight));
        io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	}

    //-------------------------------------------------------------------------------------------------

	void UI::CreateDescriptorSetLayout()
	{
        std::vector<VkDescriptorSetLayoutBinding> binding{
			VkDescriptorSetLayoutBinding{
		        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		        .descriptorCount = 1,
		        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		        .pImmutableSamplers = &_fontSampler->sampler,
			}
        };

        _descriptorSetLayout = RB::CreateDescriptorSetLayout(
            LogicalDevice::Instance->GetVkDevice(),
            static_cast<uint8_t>(binding.size()),
            binding.data()
        );
	}

	//-------------------------------------------------------------------------------------------------

    void UI::CreatePipeline()
    {
        // Constants: we are using 'vec2 offset' and 'vec2 scale' instead of a full 3d projection matrix
        std::vector<VkPushConstantRange> pushConstantRanges{};

        pushConstantRanges.emplace_back(VkPushConstantRange{
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0,
            .size = 4 * sizeof(float)
        });

        // Create Descriptor Set:
        _descriptorSetGroup = RB::CreateDescriptorSet(
            LogicalDevice::Instance->GetVkDevice(),
            _descriptorPool->descriptorPool,
            _descriptorSetLayout->descriptorSetLayout,
            1
        );


        auto const pipelineLayout = RB::CreatePipelineLayout(
            LogicalDevice::Instance->GetVkDevice(),
            1,
            &_descriptorSetLayout->descriptorSetLayout,
            static_cast<uint32_t>(pushConstantRanges.size()),
            pushConstantRanges.data()
        );

        // Vertex shader
        auto const vertexShader = RB::CreateShader(
            LogicalDevice::Instance->GetVkDevice(),
            Importer::ShaderFromSPV(
	            Alias(vertex_shader_spv),
	            VK_SHADER_STAGE_VERTEX_BIT,
	            "main"
			)
        );

        // Fragment shader
        auto const fragmentShader = RB::CreateShader(
            LogicalDevice::Instance->GetVkDevice(),
            Importer::ShaderFromSPV(
	            Alias(fragment_shader_spv),
                VK_SHADER_STAGE_FRAGMENT_BIT,
	            "main"
			)
        );

        std::vector<RT::GpuShader const*> shaderStages{
            vertexShader.get(),
            fragmentShader.get()
        };

        VkVertexInputBindingDescription vertex_binding_description{};
        vertex_binding_description.stride = sizeof(ImDrawVert);
        vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        std::vector<VkVertexInputAttributeDescription> inputAttributeDescription(3);
        inputAttributeDescription[0].location = 0;
        inputAttributeDescription[0].binding = vertex_binding_description.binding;
        inputAttributeDescription[0].format = VK_FORMAT_R32G32_SFLOAT;
        inputAttributeDescription[0].offset = offsetof(ImDrawVert, pos);
        inputAttributeDescription[1].location = 1;
        inputAttributeDescription[1].binding = vertex_binding_description.binding;
        inputAttributeDescription[1].format = VK_FORMAT_R32G32_SFLOAT;
        inputAttributeDescription[1].offset = offsetof(ImDrawVert, uv);
        inputAttributeDescription[2].location = 2;
        inputAttributeDescription[2].binding = vertex_binding_description.binding;
        inputAttributeDescription[2].format = VK_FORMAT_R8G8B8A8_UNORM;
        inputAttributeDescription[2].offset = offsetof(ImDrawVert, col);

        std::vector<VkDynamicState> const dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
        dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

        RB::CreateGraphicPipelineOptions pipelineOptions{};
        pipelineOptions.frontFace = VK_FRONT_FACE_CLOCKWISE;
        pipelineOptions.dynamicStateCreateInfo = &dynamicStateCreateInfo;
        pipelineOptions.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        pipelineOptions.depthStencil.depthTestEnable = false;
        pipelineOptions.depthStencil.depthWriteEnable = false;
        pipelineOptions.depthStencil.depthBoundsTestEnable = false;
        pipelineOptions.depthStencil.stencilTestEnable = false;
        pipelineOptions.colorBlendAttachments.blendEnable = VK_TRUE;
        pipelineOptions.colorBlendAttachments.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        pipelineOptions.colorBlendAttachments.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        pipelineOptions.colorBlendAttachments.colorBlendOp = VK_BLEND_OP_ADD;
        pipelineOptions.colorBlendAttachments.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        pipelineOptions.colorBlendAttachments.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        pipelineOptions.colorBlendAttachments.alphaBlendOp = VK_BLEND_OP_ADD;
        pipelineOptions.colorBlendAttachments.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        pipelineOptions.useStaticViewportAndScissor = false;
        pipelineOptions.cullMode = VK_CULL_MODE_NONE;
        // TODO I wish we could render ui without MaxSamplesCount
        pipelineOptions.rasterizationSamples = LogicalDevice::Instance->GetMaxSampleCount();

        auto surfaceCapabilities = LogicalDevice::Instance->GetSurfaceCapabilities();

        _pipeline = RB::CreateGraphicPipeline(
            LogicalDevice::Instance->GetVkDevice(),
            static_cast<uint8_t>(shaderStages.size()),
            shaderStages.data(),
            1,
            &vertex_binding_description,
            static_cast<uint8_t>(inputAttributeDescription.size()),
            inputAttributeDescription.data(),
            surfaceCapabilities.currentExtent,
            _displayRenderPass->GetVkRenderPass(),
            pipelineLayout,
            pipelineOptions
        );
    }

    //-------------------------------------------------------------------------------------------------

    void UI::CreateFontTexture()
    {
        // Load Fonts
        // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
        // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
        // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
        // - Read 'docs/FONTS.md' for more instructions and details.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        //io.Fonts->AddFontDefault();
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
        //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
        ImGuiIO& io = ImGui::GetIO();

        uint8_t* pixels = nullptr;
        int32_t width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        MFA_ASSERT(pixels != nullptr);
        MFA_ASSERT(width > 0);
        MFA_ASSERT(height > 0);
        size_t const components_count = 4;
        size_t const depth = 1;
        size_t const slices = 1;
        size_t const image_size = width * height * components_count * sizeof(uint8_t);

        Importer::ImportTextureOptions importTextureOptions{
            .tryToGenerateMipmaps = false
        };
        
        auto const textureAsset = Importer::InMemoryTexture(
            Alias{ pixels, image_size },
            width,
            height,
            AS::Texture::Format::UNCOMPRESSED_UNORM_R8G8B8A8_LINEAR,
            components_count,
            depth,
            slices,
            importTextureOptions
        );

        auto const device = LogicalDevice::Instance;

        auto const commandBuffer = RB::BeginSingleTimeCommand(
            device->GetVkDevice(), 
            device->GetGraphicCommandPool()
        );

        // TODO Support from in memory import of images inside importer
        auto [texture, stagingBuffer] = RB::CreateTexture(
            *textureAsset, 
            device->GetVkDevice(), 
            device->GetPhysicalDevice(),
            commandBuffer
        );

        _fontTexture = texture;

        RB::EndAndSubmitSingleTimeCommand(
			device->GetVkDevice(),
            device->GetGraphicCommandPool(),
            device->GetGraphicQueue(),
            commandBuffer
        );
    }

    //-------------------------------------------------------------------------------------------------

    void UI::BindKeyboard()
    {
        ImGuiIO& io = ImGui::GetIO();
        // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
        io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
        io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
        io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
        io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
        io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
        io.KeyMap[ImGuiKey_Insert] = SDL_SCANCODE_INSERT;
        io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
        io.KeyMap[ImGuiKey_Space] = SDL_SCANCODE_SPACE;
        io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
        io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
        io.KeyMap[ImGuiKey_KeyPadEnter] = SDL_SCANCODE_KP_ENTER;
        io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
        io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
        io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
        io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
        io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
        io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;
    }

    //-------------------------------------------------------------------------------------------------

    void UI::UpdateDescriptorSets()
    {
        // Update the Descriptor Set:
        for (auto& descriptorSet : _descriptorSetGroup.descriptorSets)
        {
            auto const imageInfo = VkDescriptorImageInfo{
                .sampler = _fontSampler->sampler,
                .imageView = _fontTexture->imageView->imageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };
            auto writeDescriptorSet = VkWriteDescriptorSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptorSet,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &imageInfo,
            };
            RB::UpdateDescriptorSets(
                LogicalDevice::Instance->GetVkDevice(),
                1,
                &writeDescriptorSet
            );
        }
    }

    //-------------------------------------------------------------------------------------------------

    void UI::UpdateMousePositionAndButtons()
    {
        auto& io = ImGui::GetIO();

        auto* window = LogicalDevice::Instance->GetWindow();

        // Set OS mouse position if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
        if (io.WantSetMousePos)
        {
            SDL_WarpMouseInWindow(window, static_cast<int32_t>(io.MousePos.x), static_cast<int32_t>(io.MousePos.y));
        }
        else
        {
            io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
        }

    	int mx, my;
        uint32_t const mouse_buttons = SDL_GetMouseState(&mx, &my);
        io.MouseDown[0] = (mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;  // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
        io.MouseDown[1] = (mouse_buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
        io.MouseDown[2] = (mouse_buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;

    	if (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS)
        {
            io.MousePos = ImVec2(static_cast<float>(mx), static_cast<float>(my));
        }
    }

    //-------------------------------------------------------------------------------------------------

    void UI::UpdateMouseCursor() const
    {
        auto& io = ImGui::GetIO();

        if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        {
            return;
        }
        ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
        if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None)
        {
            // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
            SDL_ShowCursor(SDL_FALSE);
        }
        else
        {
            // Show OS mouse cursor
            SDL_SetCursor(_mouseCursors[imgui_cursor] ? _mouseCursors[imgui_cursor] : _mouseCursors[ImGuiMouseCursor_Arrow]);
            SDL_ShowCursor(SDL_TRUE);
        }
    }

    //-------------------------------------------------------------------------------------------------

}
