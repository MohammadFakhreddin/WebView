#pragma once
#include <memory>
#include <string>

//https://stackoverflow.com/questions/4815423/how-do-i-set-the-working-directory-to-the-solution-directory
namespace MFA
{
	class Path
	{
	public:

		inline static Path * Instance = nullptr;

		static std::unique_ptr<Path> Instantiate();

		explicit Path();

		~Path();

		// Returns correct address based on platform
		[[nodiscard]]
		std::string Get(std::string const& address) const;

	private:

		std::string mAssetPath {};

	};
};