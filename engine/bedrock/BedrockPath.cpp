#include "BedrockPath.hpp"

#include "BedrockAssert.hpp"
#include "BedrockCommon.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

static bool LogCalledOnce = false;

//-------------------------------------------------------------------------------------------------

std::shared_ptr<MFA::Path> MFA::Path::Instance() {
	std::shared_ptr<Path> shared_ptr = _instance.lock();
	if (shared_ptr == nullptr)
	{
		shared_ptr = std::make_shared<Path>();
		_instance = shared_ptr;
	}
	return shared_ptr;
}

//-------------------------------------------------------------------------------------------------

MFA::Path::Path()
{
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
	    if (LogCalledOnce == false)
	    {
	        MFA_LOG_INFO("Override asset path is %s", mAssetPath.c_str());
	    }
	}
	else
	{
	    if (LogCalledOnce == false)
	    {
	        MFA_LOG_INFO("No override found, using the default directory: %s", mAssetPath.c_str());
	    }
	}

    LogCalledOnce = true;
}

//-------------------------------------------------------------------------------------------------

MFA::Path::~Path() = default;

//-------------------------------------------------------------------------------------------------

std::string MFA::Path::Get(std::string const& address) const
{
	return Get(address.c_str());
}

//-------------------------------------------------------------------------------------------------

std::string MFA::Path::Get(char const *address) const
{
    if (std::filesystem::exists(address) == true)
    {
        return address;
    }
    if (strncmp(address, "./", 2) == 0 || strncmp(address, "/", 1) == 0)
    {
        return address;
    }
    return std::filesystem::path(mAssetPath).append(address).string();
}

//-------------------------------------------------------------------------------------------------

std::string MFA::Path::Relative(char const *address) const
{
    if (strncmp(address, mAssetPath.c_str(), mAssetPath.size()) == 0)
    {
        return std::string(address).substr(mAssetPath.size());
    }
    return address;
}

//-------------------------------------------------------------------------------------------------
