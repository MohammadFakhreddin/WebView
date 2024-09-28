#include "BedrockFile.hpp"

#include <filesystem>
#include <fstream>
#include <vector>

#include "BedrockAssert.hpp"
#include "BedrockLog.hpp"


namespace MFA::File
{
    std::shared_ptr<Blob> Read(std::string const & path)
    {
        if (MFA_VERIFY(std::filesystem::exists(path)))
		{
            std::ifstream file(path, std::ios::binary);
			std::string line{};
			
            /*std::string data {};
            while (std::getline(file, line))
			{
                data += line;
			}*/
            std::shared_ptr<Blob> blob = nullptr;
            if (file.good())
            {
                std::vector<uint8_t> data((
                    std::istreambuf_iterator<char>(file)),
                    (std::istreambuf_iterator<char>())
                );

                blob = Memory::Alloc(data.data(), data.size());
            }

            file.close();

            return blob;
        }
        return nullptr;
    }
}