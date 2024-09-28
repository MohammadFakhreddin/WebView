#include "Transform.hpp"

#include "BedrockMath.hpp"
#include "imgui.h"

namespace MFA
{
    
	//-------------------------------------------------------------------------------------------------

    Transform::Transform() = default;
	
	//-------------------------------------------------------------------------------------------------

	bool Transform::SetEulerAngles(glm::vec3 const & eulerAngles)
	{
		mIsLocalTransformDirty |= mLocalRotation.SetEulerAngles(eulerAngles);
		if (mIsLocalTransformDirty == true)
		{
			SetGlobalTransformDirty();
			return true;
		}
		return false;
	}

	//-------------------------------------------------------------------------------------------------

	bool Transform::SetLocalQuaternion(glm::quat const & quaternion)
	{
		mIsLocalTransformDirty |= mLocalRotation.SetQuaternion(quaternion);
		if (mIsLocalTransformDirty == true)
		{
			SetGlobalTransformDirty();
			return true;
		}
		return false;
	}

	//-------------------------------------------------------------------------------------------------

	glm::mat4 const& Transform::LocalTransform()
	{
		if (mIsLocalTransformDirty == true)
		{
			mLocalTransform = mLocalExtraTransform * 
				Math::Translate(mLocalPosition) * 
				mLocalRotation.GetMatrix() *
				Math::Scale(mLocalScale);

			mIsLocalTransformDirty = false;
		}
		return mLocalTransform;
	}

	//-------------------------------------------------------------------------------------------------

	glm::mat4 const& Transform::GlobalTransform()
	{
		if (mGlobalTransformDirty == true)
		{
			auto const & localTransform = LocalTransform();
			mGlobalTransform = glm::identity<glm::mat4>();
			if (mParent != nullptr)
			{
				mGlobalTransform *= mParent->GlobalTransform();
			}
			mGlobalTransform *= localTransform;
			mGlobalTransformDirty = false;
		}
		return mGlobalTransform;
	}

	//-------------------------------------------------------------------------------------------------

	Transform * Transform::Parent() const
	{
		return mParent;
	}

	//-------------------------------------------------------------------------------------------------

	std::set<Transform*> const& Transform::Children()
	{
		return mChildren;
	}

	//-------------------------------------------------------------------------------------------------

	void Transform::SetParent(Transform* parent)
	{
		if (mParent == parent)
		{
			return;
		}
		if (mParent != nullptr)
		{
			mParent->RemoveChild(this);
		}
		mParent = parent;
		mGlobalTransformDirty = true;
		if (mParent != nullptr)
		{
			mParent->AddChild(this);
		}
	}

	//-------------------------------------------------------------------------------------------------

	void Transform::AddChild(Transform* child)
	{
		if (mChildren.contains(child) == false)
		{
			mChildren.emplace(child);
			child->SetParent(this);
		}
	}

	//-------------------------------------------------------------------------------------------------

	void Transform::RemoveChild(Transform* child)
	{
		if (mChildren.contains(child) == true)
		{
			mChildren.erase(child);
			child->SetParent(nullptr);
		}
	}

	//-------------------------------------------------------------------------------------------------

	glm::vec3 const & Transform::Forward()
	{
		if (mForwardDirty == true)
		{
			mForward = glm::normalize(GlobalTransform() * Math::ForwardVec4W0);
			mForwardDirty = false;
		}
		return mForward;
	}

	//-------------------------------------------------------------------------------------------------

	glm::vec3 const & Transform::Right()
	{
		if (mRightDirty == true)
		{
			mRight = glm::normalize(GlobalTransform() * Math::RightVec4W0);
			mRightDirty = false;
		}
		return mRight;
	}

	//-------------------------------------------------------------------------------------------------

	glm::vec3 const & Transform::Up()
	{
		if (mUpDirty == true)
		{
			mUp = glm::normalize(GlobalTransform() * Math::UpVec4W0);
			mUpDirty = false;
		}
		return mUp;
	}

	//-------------------------------------------------------------------------------------------------

	glm::vec3 const & Transform::GlobalPosition()
	{
		if (mGlobalPositionDirty == true)
		{
			mGlobalPosition = GlobalTransform() * glm::vec4{0.0, 0.0, 0.0, 1.0f};
			mGlobalPositionDirty = false;
		}
		return mGlobalPosition;
	}

	//-------------------------------------------------------------------------------------------------

	Rotation const & Transform::GlobalRotation()
	{
		if (mGlobalRotationDirty == true)
		{
			mGlobalRotation.SetQuaternion(glm::quatLookAt(Forward(),Math::UpVec3));
			mGlobalRotationDirty = false;
		}
		return mGlobalRotation;
	}

	//-------------------------------------------------------------------------------------------------

	void Transform::DebugUI()
	{
		bool isDirty = false;

		if(ImGui::InputFloat3("Position",reinterpret_cast<float *>(&mLocalPosition)))
		{
			isDirty = true;
		}

		{
			glm::vec3 eulerAngles = mLocalRotation.GetEulerAngles();
			if(ImGui::InputFloat3("Euler angles",reinterpret_cast<float *>(&eulerAngles)))
			{
				mLocalRotation.SetEulerAngles(eulerAngles);
				isDirty = true;
			}
		}

		if (ImGui::InputFloat3("Scale", reinterpret_cast<float *>(&mLocalScale)))
		{
			isDirty = true;
		}

		if (isDirty == true)
		{
			SetLocalTransformDirty();
		}
	}

	//-------------------------------------------------------------------------------------------------

	void Transform::SetLocalTransformDirty()
	{
		mIsLocalTransformDirty = true;
		SetGlobalTransformDirty();
	}

	//-------------------------------------------------------------------------------------------------

	void Transform::SetGlobalTransformDirty()
	{
		mGlobalTransformDirty = true;
		mGlobalPositionDirty = true;
		mGlobalRotationDirty = true;
		mForwardDirty = true;
		mUpDirty = true;
		mRightDirty = true;

		for(auto * child : mChildren)
		{
			child->SetGlobalTransformDirty();
		}
	}

	//-------------------------------------------------------------------------------------------------

}
