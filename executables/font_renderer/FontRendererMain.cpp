#include "BedrockFile.hpp"
#include "BedrockPath.hpp"
#include "FontParser.hpp"

using namespace MFA;
using namespace Shared;
// TODO: Start writting document for your project. Share a video clip about fire particle.
// The idea is to read and display font on screen :)
int main()
{
    auto const path = Path::Instance();
    // auto const fontPath = path->Get("fonts/PublicSans-Bold.ttf");
    auto const fontPath = path->Get("fonts/JetBrains-Mono/JetBrainsMonoNL-Regular.ttf");
    MFA_ASSERT(std::filesystem::exists(fontPath) == true);
    auto const fontData = FontParser::Parse(File::Read(fontPath));
    return 0;
}