#include "BedrockPath.hpp"

#include "BedrockAssert.hpp"
#include "BedrockPath.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

#define VALUE(string) #string
#define TO_LITERAL(string) VALUE(string)

//-------------------------------------------------------------------------------------------------

std::unique_ptr<MFA::Path> MFA::Path::Instantiate()
{
	return std::make_unique<Path>();
}

//-------------------------------------------------------------------------------------------------

MFA::Path::Path()
{
	MFA_ASSERT(Instance == nullptr);
	Instance = this;

#if defined(ASSET_DIR)
	mAssetPath = std::filesystem::absolute(std::string(TO_LITERAL(ASSET_DIR))).string();
#endif
	
	static constexpr char const * OVERRIDE_ASSET_PATH = "./asset_dir.txt";
	if (std::filesystem::exists(OVERRIDE_ASSET_PATH))
	{
		std::ifstream nameFileout{};
		mAssetPath.clear();

		nameFileout.open(OVERRIDE_ASSET_PATH);
		while (nameFileout >> mAssetPath)
		{
			std::cout << mAssetPath;
		}
		nameFileout.close();
		MFA_LOG_INFO("Override asset path is %s", mAssetPath.c_str());
	}
	else
	{
		MFA_LOG_INFO("No override found, using the default directory: %s", mAssetPath.c_str());
	}

}

//-------------------------------------------------------------------------------------------------

MFA::Path::~Path()
{
	MFA_ASSERT(Instance != nullptr);
	Instance = nullptr;
}

//-------------------------------------------------------------------------------------------------

std::string MFA::Path::Get(std::string const& address) const
{
	return std::filesystem::path(mAssetPath).append(address).string();
}

//-------------------------------------------------------------------------------------------------
