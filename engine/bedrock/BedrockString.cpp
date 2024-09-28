#include "BedrockString.hpp"

namespace MFA::String
{
    // https://en.cppreference.com/w/cpp/string/byte/tolower
    std::string ToLowerCase(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(), 
            [](unsigned char c){ return std::tolower(c); } // correct
        );
        return s;
    }

    std::vector<std::string> Split(std::string const & text, std::string const & separator)
    {
        std::regex regexz(separator);
        std::vector<std::string> list(
            std::sregex_token_iterator(text.begin(), text.end(), regexz, -1), 
            std::sregex_token_iterator()
        );
        return list;
    }
}
