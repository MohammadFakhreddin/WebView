#include "PerspectiveCamera.hpp"

#include "BedrockMath.hpp"
#include "LogicalDevice.hpp"

#include "imgui.h"

namespace MFA
{

	//-------------------------------------------------------------------------------------------------

	PerspectiveCamera::PerspectiveCamera()
	{
		_resizeListener = LogicalDevice::Instance->ResizeEventSignal1.Register([this]()
		{
			SetProjectionDirty();
		});
	}

	//-------------------------------------------------------------------------------------------------

	PerspectiveCamera::~PerspectiveCamera()
	{
		LogicalDevice::Instance->ResizeEventSignal1.UnRegister(_resizeListener);
	}

	//-------------------------------------------------------------------------------------------------

	glm::mat4 const & PerspectiveCamera::ViewProjection()
	{
		if (_isViewProjectionDirty == true)
		{
			_viewProjMat = Projection() * View();
			_isViewProjectionDirty = false;
		}
		return _viewProjMat;
	}

	//-------------------------------------------------------------------------------------------------

	glm::mat4 const & PerspectiveCamera::View()
	{
		if (_isViewDirty == true)
		{
			auto const invTran = glm::translate(glm::identity<glm::mat4>(), -_transform.GetLocalPosition());
			auto const invRot = glm::transpose(_transform.GetLocalRotation().GetMatrix());
			_viewMat = invRot * invTran;
			_isViewDirty = false;
		}
		return _viewMat;
	}

	//-------------------------------------------------------------------------------------------------

	glm::mat4 const & PerspectiveCamera::Projection()
	{
		if (_isProjectionDirty == true)
		{
			auto const extent = LogicalDevice::Instance->GetSurfaceCapabilities().currentExtent;
			float const aspectRatio = static_cast<float>(extent.width) / static_cast<float>(extent.height);

			Math::PerspectiveProjection(_projMat, aspectRatio, _fovDeg, _nearPlane, _farPlane);
			_isProjectionDirty = false;
		}
		return _projMat;
	}

	//-------------------------------------------------------------------------------------------------

	void PerspectiveCamera::SetLocalPosition(glm::vec3 const& localPosition)
	{
		bool const changed = _transform.SetLocalPosition(localPosition);
		if (changed)
		{
			SetViewDirty();
		}
	}

	//-------------------------------------------------------------------------------------------------

	void PerspectiveCamera::SetLocalRotation(Rotation const& localRotation)
	{
		bool const changed = _transform.SetLocalRotation(localRotation);
		if(changed)
		{
			SetViewDirty();
		}
	}

	//-------------------------------------------------------------------------------------------------

	void PerspectiveCamera::SetEulerAngles(glm::vec3 const& eulerAngles)
	{
		bool const changed = _transform.SetEulerAngles(eulerAngles);
		if(changed)
		{
			SetViewDirty();
		}
	}

	//-------------------------------------------------------------------------------------------------

	void PerspectiveCamera::SetLocalQuaternion(glm::quat const& quaternion)
	{
		bool const changed = _transform.SetLocalQuaternion(quaternion);
		if(changed)
		{
			SetViewDirty();
		}
	}

	//-------------------------------------------------------------------------------------------------

	glm::vec3 const& PerspectiveCamera::LocalPosition()
	{
		return _transform.GetLocalPosition();
	}

	//-------------------------------------------------------------------------------------------------

	Rotation const& PerspectiveCamera::LocalRotation()
	{
		return _transform.GetLocalRotation();
	}

	//-------------------------------------------------------------------------------------------------

	glm::vec3 const& PerspectiveCamera::Forward()
	{
		return _transform.Forward();
	}

	//-------------------------------------------------------------------------------------------------

	glm::vec3 const& PerspectiveCamera::Right()
	{
		return _transform.Right();
	}

	//-------------------------------------------------------------------------------------------------

	glm::vec3 const& PerspectiveCamera::Up()
	{
		return _transform.Up();
	}

	//-------------------------------------------------------------------------------------------------

	glm::vec3 const& PerspectiveCamera::GlobalPosition()
	{
		return _transform.GlobalPosition();
	}

	//-------------------------------------------------------------------------------------------------

	Rotation const& PerspectiveCamera::GlobalRotation()
	{
		return _transform.GlobalRotation();
	}

	//-------------------------------------------------------------------------------------------------

	void PerspectiveCamera::SetProjectionDirty()
	{
		_isProjectionDirty = true;
		_isViewProjectionDirty = true;
	}

	//-------------------------------------------------------------------------------------------------

	void PerspectiveCamera::SetViewDirty()
	{
		_isViewDirty = true;
		_isViewProjectionDirty = true;
	}

	//-------------------------------------------------------------------------------------------------

	bool PerspectiveCamera::IsDirty() const
	{
		return _isViewDirty == true || _isProjectionDirty == true;
	}

	//-------------------------------------------------------------------------------------------------

	void PerspectiveCamera::Debug_UI()
	{
		if (ImGui::InputFloat("Fov degree", &_fovDeg))
		{
			SetProjectionDirty();
		}
		
		if (ImGui::InputFloat("Far plane", &_farPlane))
		{
			SetProjectionDirty();
		}

		if (ImGui::InputFloat("Near plane", &_nearPlane))
		{
			SetProjectionDirty();
		}

		_transform.DebugUI();
	}
	
	//-------------------------------------------------------------------------------------------------

}

