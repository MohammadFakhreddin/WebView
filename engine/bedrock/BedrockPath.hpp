#pragma once

#include <filesystem>
#include <memory>
#include <string>

//https://stackoverflow.com/questions/4815423/how-do-i-set-the-working-directory-to-the-solution-directory
namespace MFA
{
    class Path
    {
    public:

        static std::shared_ptr<Path> Instance();

        explicit Path();

        ~Path();

        // Returns correct address based on platform
        [[nodiscard]]
        std::string Get(std::string const& address) const;

        [[nodiscard]]
        std::string Get(char const * address) const;

        std::string Relative(char const * address) const;

        [[nodiscard]]
        std::string const & AssetPath() const {return mAssetPath;}

    private:

        inline static std::weak_ptr<Path> _instance {};
        std::string mAssetPath {};

    };
};