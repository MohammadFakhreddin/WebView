#pragma once

#include "AssetGLTF_Mesh.hpp"
#include "AssetTexture.hpp"

#include <memory>
#include <vector>

namespace MFA::Asset::GLTF
{
	class Model
	{
	public:
		std::shared_ptr<Mesh> mesh{};
		std::vector<std::shared_ptr<AS::Texture>> textures{};
	};
}
