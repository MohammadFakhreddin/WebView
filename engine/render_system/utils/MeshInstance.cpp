#include "MeshInstance.hpp"

#include "Transform.hpp"
#include "utils/MeshRenderer.hpp"

namespace MFA
{

	//-------------------------------------------------------------------------------------------------

	MeshInstance::MeshInstance() = default;

	//-------------------------------------------------------------------------------------------------

	MeshInstance::MeshInstance(MeshRenderer const& meshRenderer)
	{
		auto const & nodes = meshRenderer.GetNodes();
		_nodes = nodes;
		for (int i = 0; i < _nodes.size(); ++i)
		{
			auto const & otherT = nodes[i].transform;
			Transform myT{};
			myT.SetLocalPosition(otherT.GetLocalPosition());
			myT.SetLocalRotation(otherT.GetLocalRotation());
			myT.SetLocalScale(otherT.GetLocalScale());
			_nodes[i].transform = myT;
		}
		for (int i = 0; i < nodes.size(); ++i)
		{
			if (nodes[i].HasParent() == true)
			{
				_nodes[i].transform.SetParent(&_nodes[nodes[i].parent].transform);
			}
			else
			{
				_nodes[i].transform.SetParent(&_transform);
			}
		}
	}

	//-------------------------------------------------------------------------------------------------

	MeshInstance::Node* MeshInstance::FindNode(std::string const& name)
	{
		for (auto & node : _nodes)
		{
			if (node.name == name)
			{
				return &node;
			}
		}
		return nullptr;
	}

	//-------------------------------------------------------------------------------------------------

	std::vector<Asset::GLTF::Node> & MeshInstance::GetNodes()
	{
		return _nodes;
	}

	//-------------------------------------------------------------------------------------------------

	Transform & MeshInstance::GetTransform()
	{
		return _transform;
	}

	//-------------------------------------------------------------------------------------------------

	void MeshInstance::SetTransform(const Transform& transform)
	{
		_transform = transform;
	}

	//-------------------------------------------------------------------------------------------------

}
