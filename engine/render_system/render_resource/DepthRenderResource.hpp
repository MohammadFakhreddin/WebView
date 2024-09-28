#pragma once

#include "RenderResource.hpp"
#include "RenderTypes.hpp"

#include <memory>
#include <vector>

namespace MFA
{
    class DepthRenderResource final : public RenderResource
    {
    public:

        explicit DepthRenderResource();

        ~DepthRenderResource() override;

        DepthRenderResource(DepthRenderResource const&) noexcept = delete;
        DepthRenderResource(DepthRenderResource&&) noexcept = delete;
        DepthRenderResource& operator = (DepthRenderResource const&) noexcept = delete;
        DepthRenderResource& operator = (DepthRenderResource&&) noexcept = delete;

        [[nodiscard]]
        RT::DepthImageGroup const& GetDepthImage(int index) const;

        [[nodiscard]]
        RT::DepthImageGroup const& GetDepthImage(RT::CommandRecordState const& recordState) const;

        void CreateDepthImages();

    protected:

        void OnResize() override;

    private:

        std::vector<std::shared_ptr<RT::DepthImageGroup>> mDepthImageGroupList{};

    };
}
