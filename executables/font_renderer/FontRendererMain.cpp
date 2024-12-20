#include "BedrockFile.hpp"
#include "BedrockPath.hpp"
#include "FontParser.hpp"

using namespace MFA;
using namespace Shared;

// The idea is to read and display font on screen :)
int main()
{
    auto const path = Path::Instance();
    auto const fontData = FontParser::Parse(MFA::File::Read(path->Get("fonts/PublicSans-Bold.ttf")));
    return 0;
}