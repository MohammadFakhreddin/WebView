#pragma once

#include "BedrockMemory.hpp"

#include <memory>
#include <string>
#include <vulkan/vulkan_core.h>

namespace MFA::Asset
{
    class Shader final
    {
    public:

        explicit Shader(
            std::string entryPoint_,
            VkShaderStageFlagBits const stage_,
            std::shared_ptr<Blob> compiledShaderCode_
        )
            : entryPoint(std::move(entryPoint_))
            , stage(stage_)
            , compiledShaderCode(std::move(compiledShaderCode_))
        {}

        ~Shader() = default;

        Shader(Shader const &) noexcept = delete;
        Shader(Shader &&) noexcept = delete;
        Shader & operator= (Shader const & rhs) noexcept = delete;
        Shader & operator= (Shader && rhs) noexcept = delete;

        std::string const entryPoint;     // Ex: main
        VkShaderStageFlagBits const stage;
        std::shared_ptr<Blob> const compiledShaderCode;

    };
};

namespace MFA
{
    namespace AS = Asset;
};
