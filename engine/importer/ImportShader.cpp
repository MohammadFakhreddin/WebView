#include "ImportShader.hpp"

#include "BedrockAssert.hpp"
#include "BedrockLog.hpp"
#include "BedrockFile.hpp"

namespace MFA::Importer
{

	//-------------------------------------------------------------------------------------------------

	std::shared_ptr<AS::Shader> ShaderFromSPV(
		std::string const& path, 
		VkShaderStageFlagBits const stage,
		std::string const& entryPoint
	)
	{
		std::shared_ptr<AS::Shader> shader = nullptr;
		auto buffer = File::Read(path);
		if (buffer != nullptr)
		{
			shader = std::make_shared<AS::Shader>(entryPoint, stage, buffer);
		}
		else
		{
			MFA_LOG_WARN("Importing shader from path %s failed\n", path.c_str());
		}
		return shader;
	}

	//-------------------------------------------------------------------------------------------------

	std::shared_ptr<AS::Shader> ShaderFromSPV(
		BaseBlob const& dataMemory,
		VkShaderStageFlagBits stage,
		std::string const& entryPoint
	)
	{
		std::shared_ptr<AS::Shader> shader = nullptr;
		if (dataMemory.IsValid())
		{
			std::shared_ptr<Blob> buffer = std::make_shared<Blob>(dataMemory.Ptr(), dataMemory.Len());
			shader = std::make_shared<AS::Shader>(entryPoint, stage, buffer);
		}
		else
		{
			MFA_LOG_WARN("Failed to create shader from memory");
		}
		return shader;
	}

	//-------------------------------------------------------------------------------------------------

    bool CompileShaderToSPV(
        std::string const & inputPath,
        std::string const & outputPath,
        std::string const & stage
    )
	{
		std::string command = "";
		MFA_STRING(
			command,
			"glslc -g -fshader-stage=%s \"%s\" -o \"%s\" -std=450core",
			stage.c_str(),
			inputPath.c_str(),
			outputPath.c_str()
		);
		auto const result = std::system(command.c_str());
		return result == 0;
	}

	//-------------------------------------------------------------------------------------------------

}
