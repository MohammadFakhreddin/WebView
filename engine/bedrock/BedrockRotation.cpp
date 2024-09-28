#include "BedrockRotation.hpp"

#include "BedrockMath.hpp"
#include "BedrockMemory.hpp"

namespace MFA
{

    //-------------------------------------------------------------------------------------------------

    Rotation::Rotation() = default;

    //-------------------------------------------------------------------------------------------------

    Rotation::Rotation(glm::vec3 const & eulerAngles)
        : mQuaternion(Math::ToQuat(eulerAngles))
        , mEulerAngles(eulerAngles)
        , mMatrix(glm::toMat4(mQuaternion))
    {}

    //-------------------------------------------------------------------------------------------------

    Rotation::Rotation(glm::quat const & quaternion)
        : mQuaternion(quaternion)
        , mEulerAngles(Math::ToEulerAngles(quaternion))
        , mMatrix(glm::toMat4(quaternion))
    {}

    //-------------------------------------------------------------------------------------------------

    glm::vec3 const & Rotation::GetEulerAngles() const
    {
        return mEulerAngles;
    }

    //-------------------------------------------------------------------------------------------------

    glm::quat const & Rotation::GetQuaternion() const
    {
        return mQuaternion;
    }

    //-------------------------------------------------------------------------------------------------

    glm::mat4 const & Rotation::GetMatrix() const
    {
        return mMatrix;
    }

    //-------------------------------------------------------------------------------------------------

    bool Rotation::SetEulerAngles(glm::vec3 const & eulerAngles)
    {
        if (Memory::IsEqual(mEulerAngles, eulerAngles))
        {
            return false;
        }
        mEulerAngles = eulerAngles;
        mQuaternion = Math::ToQuat(eulerAngles);
        mMatrix = glm::toMat4(mQuaternion);
        return true;
    }

    //-------------------------------------------------------------------------------------------------

    bool Rotation::SetQuaternion(glm::quat const & quaternion)
    {
        if (Math::IsEqual(mQuaternion, quaternion))
        {
            return false;
        }
        mQuaternion = quaternion;
        mEulerAngles = Math::ToEulerAngles(quaternion);
        mMatrix = glm::toMat4(mQuaternion);
        return true;
    }

    //-------------------------------------------------------------------------------------------------
    
    bool Rotation::operator==(glm::vec3 const & eulerAngles) const
    {
        return Memory::IsEqual(mEulerAngles, eulerAngles);
    }

    //-------------------------------------------------------------------------------------------------

    bool Rotation::operator==(glm::quat const & quaternion) const
    {
        return Math::IsEqual(mQuaternion, quaternion);
    }

    //-------------------------------------------------------------------------------------------------

    bool Rotation::operator==(float eulerAngles[3]) const
    {
        return Memory::IsEqual<3>(mEulerAngles, eulerAngles);
    }

    //-------------------------------------------------------------------------------------------------

    bool Rotation::operator==(Rotation const & rotation) const
    {
        return Math::IsEqual(mQuaternion, rotation.GetQuaternion());
    }

    //-------------------------------------------------------------------------------------------------

    bool Rotation::operator!=(glm::vec3 const & eulerAngles) const
    {
        return !Memory::IsEqual(mEulerAngles, eulerAngles);
    }

    //-------------------------------------------------------------------------------------------------

    bool Rotation::operator!=(glm::quat const & quaternion) const
    {
        return !Math::IsEqual(mQuaternion, quaternion);
    }

    //-------------------------------------------------------------------------------------------------

    bool Rotation::operator!=(float eulerAngles[3]) const
    {
        return !Memory::IsEqual<3>(mEulerAngles, eulerAngles);
    }

    //-------------------------------------------------------------------------------------------------

    bool Rotation::operator!=(Rotation const & rotation) const
    {
        return !Math::IsEqual(mQuaternion, rotation.GetQuaternion());
    }

    //-------------------------------------------------------------------------------------------------

    Rotation & Rotation::operator=(float eulerAngles[3])
    {
        if (*this != eulerAngles) 
        {
            Memory::Copy<3>(mEulerAngles, eulerAngles);
            mQuaternion = Math::ToQuat(mEulerAngles);
            mMatrix = glm::toMat4(mQuaternion);
        }
        return *this;
    }

    //-------------------------------------------------------------------------------------------------

    Rotation & Rotation::operator=(glm::vec3 const & eulerAngles)
    {
        if (*this != eulerAngles)
        {
            Memory::Copy(mEulerAngles, eulerAngles);
            mQuaternion = Math::ToQuat(mEulerAngles);
            mMatrix = glm::toMat4(mQuaternion);
        }
        return *this;
    }

    //-------------------------------------------------------------------------------------------------

    Rotation & Rotation::operator=(glm::quat const & quaternion)
    {
        if (*this != quaternion)
        {
            Memory::Copy(mQuaternion, quaternion);
            mEulerAngles = Math::ToEulerAngles(quaternion);
            mMatrix = glm::toMat4(mQuaternion);
        }
        return *this;
    }

    //-------------------------------------------------------------------------------------------------

}
