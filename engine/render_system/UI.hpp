#pragma once

#include "BedrockSignal.hpp"
#include "RenderBackend.hpp"
#include "render_pass/DisplayRenderPass.hpp"

#include "imgui.h"

#include <memory>

namespace MFA
{

	class UI
	{
	public:

		explicit UI(std::shared_ptr<DisplayRenderPass> displayRenderPass);

		~UI();

        void Update();

        bool Render(
            RT::CommandRecordState& recordState,
            float deltaTimeInSec
        );

        [[nodiscard]]
        bool HasFocus() const;

        void BeginWindow(std::string const& windowName);

        void EndWindow();

        inline static UI* Instance = nullptr;

        Signal<> UpdateSignal{};

	private:

        struct PushConstants
        {
            float scale[2];
            float translate[2];
        };

        void OnResize();

        void CreateDescriptorSetLayout();

        void CreatePipeline();

        void CreateFontTexture();

		static void BindKeyboard();

        void UpdateDescriptorSets();

		static void UpdateMousePositionAndButtons();

        void UpdateMouseCursor() const;

	private:

        std::shared_ptr<DisplayRenderPass> _displayRenderPass{};

        std::shared_ptr<RT::SamplerGroup> _fontSampler{};
        std::shared_ptr<RT::DescriptorSetLayoutGroup> _descriptorSetLayout{};
        std::shared_ptr<RT::DescriptorPool> _descriptorPool{};
        RT::DescriptorSetGroup _descriptorSetGroup{};
        std::shared_ptr<RT::PipelineGroup> _pipeline{};
        std::shared_ptr<RT::GpuTexture> _fontTexture{};
        bool _hasFocus = false;
        std::vector<std::shared_ptr<RT::BufferAndMemory>> _vertexBuffers{};
        std::vector<std::shared_ptr<RT::BufferAndMemory>> _indexBuffers{};
        SDL_Cursor* _mouseCursors[ImGuiMouseCursor_COUNT]{};
        int _eventWatchId = -1;
        SignalId _resizeSignalId = SignalIdInvalid;
	};

}
