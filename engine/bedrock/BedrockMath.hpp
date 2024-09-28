#pragma once


#include <random>
#include <tuple>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/norm.hpp>

namespace MFA::Math 
{

    inline static constexpr glm::vec4 ForwardVec4W1{ 0.0f, 0.0f, 1.0f, 1.0f };
    inline static constexpr glm::vec4 RightVec4W1{ 1.0f, 0.0f, 0.0f, 1.0f };
    inline static constexpr glm::vec4 UpVec4W1{ 0.0f, 1.0f, 0.0f, 1.0f };

    inline static constexpr glm::vec4 ForwardVec4W0{ 0.0f, 0.0f, 1.0f, 0.0f };
    inline static constexpr glm::vec4 RightVec4W0{ 1.0f, 0.0f, 0.0f, 0.0f };
    inline static constexpr glm::vec4 UpVec4W0{ 0.0f, 1.0f, 0.0f, 0.0f };

    inline static constexpr glm::vec3 ForwardVec3{ 0.0f, 0.0f, 1.0f };
    inline static constexpr glm::vec3 RightVec3{ 1.0f, 0.0f, 0.0f };
    inline static constexpr glm::vec3 UpVec3{ 0.0f, 1.0f, 0.0f };

    inline static constexpr glm::dvec4 DForwardVec4W1{ 0.0f, 0.0f, 1.0f, 1.0f };
    inline static constexpr glm::dvec4 DRightVec4W1{ 1.0f, 0.0f, 0.0f, 1.0f };
    inline static constexpr glm::dvec4 DUpVec4W1{ 0.0f, 1.0f, 0.0f, 1.0f };

    inline static constexpr glm::dvec4 DForwardVec4W0{ 0.0f, 0.0f, 1.0f, 0.0f };
    inline static constexpr glm::dvec4 DRightVec4W0{ 1.0f, 0.0f, 0.0f, 0.0f };
    inline static constexpr glm::dvec4 DUpVec4W0{ 0.0f, 1.0f, 0.0f, 0.0f };

    inline static constexpr glm::dvec3 DForwardVec3{ 0.0f, 0.0f, 1.0f };
    inline static constexpr glm::dvec3 DRightVec3{ 1.0f, 0.0f, 0.0f };
    inline static constexpr glm::dvec3 DUpVec3{ 0.0f, 1.0f, 0.0f };

    template<typename T>
    T Random(T min, T max)
    {
        float const fMin = static_cast<float>(min);
        float const fMax = static_cast<float>(max);
        float value = (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) * (fMax - fMin) + fMin;
        return static_cast<T>(value);
    }

    [[nodiscard]]
    glm::quat ToQuat(float xDeg, float yDeg, float zDeg);

    [[nodiscard]]
    glm::quat ToQuat(glm::vec3 const & eulerAngles);

    [[nodiscard]]
    glm::vec3 ToEulerAngles(glm::quat const & quaternion);

    void RotateWithEulerAngle(glm::mat4 & inOutTransform, float eulerAngles[3]);

    void RotateWithEulerAngle(glm::mat4 & inOutTransform, glm::vec3 const & eulerAngles);

    [[nodiscard]]
    glm::mat4 RotateWithEulerAngle(glm::vec3 const & eulerAngles);

    void RotateWithRadians(glm::mat4 & inOutTransform, float radians[3]);

    void RotateWithRadians(glm::mat4 & inOutTransform, glm::vec3 radians);

    void Scale(glm::mat4 & transform, float scale[3]);

    void Scale(glm::mat4 & transform, glm::vec3 const & scale);

    void Scale(glm::mat4& transform, float scale);

    glm::mat4 Scale(glm::vec3 const & scale);

    glm::mat4 Scale(float scale);

    glm::dmat4 Scale(glm::dvec3 const& scale);

    void Translate(glm::mat4 & transform, float distance[3]);

    void Translate(glm::mat4 & transform, glm::vec3 const & distance);

    glm::mat4 Translate(glm::vec3 const& distance);

    glm::dmat4 Translate(glm::dvec3 const& distance);

    void PerspectiveProjection(
        float outMatrix[16],
        float aspectRatio,
        float fieldOfView,          // In degree
        float nearPlane,
        float farPlane 
    );

    void PerspectiveProjection(
        glm::mat4 & outMatrix,
        float aspectRatio,
        float fieldOfView,          // In degree
        float nearPlane,
        float farPlane 
    );

    void OrthographicProjection(
        glm::mat4 & outMatrix,
        float leftPlane,
        float rightPlane,
        float bottomPlane,
        float topPlane,
        float nearPlane,
        float farPlane
    );

    template<typename T>
    bool IsNearZero(T const & glmVec) {
        return glm::length2(glmVec) < glm::epsilon<float>();
    }

    [[nodiscard]]
    float UnSignedAngle(glm::vec3 const& a, glm::vec3 const& b);

    // Returns angle of 2 quaternions in radians
    [[nodiscard]]
    float UnSignedAngle(glm::quat const & a, glm::quat const & b);

    [[nodiscard]]
    bool IsEqualUsingDot(float const dot);

    [[nodiscard]]
    bool IsEqual(glm::quat const & a, glm::quat const & b);

    // Vectors should be normalized
    [[nodiscard]]
    glm::quat FindRotation(glm::vec3 const & from, glm::vec3 const & to);

    [[nodiscard]]
    glm::vec3 MoveTowards(
        glm::vec3 const & from,
        glm::vec3 const & to,
        float maxDistance
    );

    /***
     *
     * @param from Quaternion that we start from
     * @param to Quaternion that we want to reach
     * @param maxDegreeDelta Max degrees in radian that we can move
     ***/
    glm::quat RotateTowards(
        glm::quat const & from,
        glm::quat const & to,
        float maxDegreeDelta
    );

    template<typename T>
    [[nodiscard]]
    T ACosSafe(T const value)
    {
        if (value <= -static_cast<T>(1.0))
        {
            return glm::pi<T>();
        }

        if (value >= static_cast<T>(1.0))
        {
            return static_cast<T>(0.0);
        }

        return std::acos(value);
    }

    [[nodiscard]]
    float SignedAngle2d(glm::vec2 const& from, glm::vec2 const& to);

    [[nodiscard]]
    float SignedAngle(glm::vec3 const& from, glm::vec3 const& to);

    [[nodiscard]]
    glm::mat4 ChangeOfBasis(glm::vec3 const & biTangent, glm::vec3 const & normal, glm::vec3 const & tangent);

	[[nodiscard]]
	glm::mat3 TriangleTransform2d(
	    glm::vec2 const& localP1, glm::vec2 const & localP2, glm::vec2 const & localP3, 
	    glm::vec2 const& worldP1, glm::vec2 const & worldP2, glm::vec2 const & worldP3
	);

    [[nodiscard]]
    glm::vec4 WorldSpaceToProjectedSpace(glm::vec4 const& worldPosition, glm::mat4 const& viewProjection);

    [[nodiscard]]
    glm::vec2 ScreenSpaceToProjectedSpace(glm::vec2 const& point, float screenWidth, float screenHeight);

    [[nodiscard]]
    bool IsValid(float value);

    [[nodiscard]]
    bool IsValid(double value);

    [[nodiscard]]
    bool IsValid(glm::dvec3 const & vec);

    [[nodiscard]]
    bool IsValid(glm::vec3 const & vec);

    [[nodiscard]]
    double CalculateVolume(
        glm::dvec3 const& p0,
        glm::dvec3 const& p1,
        glm::dvec3 const& p2,
        glm::dvec3 const& p3
    );

    [[nodiscard]]
    std::vector<std::tuple<int, int>> Rasterize(int xs, int ys, int xe, int ye);

    [[nodiscard]]
    std::vector<std::tuple<int, int, int>> Rasterize(
        int xs, int ys, int zs, 
        int xe, int ye, int ze
    );

    [[nodiscard]]
    glm::dvec3 ToLocalCoordinate(
        glm::dvec3 const& input,

        glm::dvec3 const& center,
        glm::dvec3 const& biTan,                // x
        glm::dvec3 const& normal,               // y
        glm::dvec3 const& tan                   // z
    );

    void GenerateLocalCoordinate(
        glm::dvec3 const& p0,
        glm::dvec3 const& p1,
        glm::dvec3 const& p2,

        glm::dvec3 & outCenter,
        glm::dvec3 & outBiTan,                // x
        glm::dvec3 & outNormal,               // y
        glm::dvec3 & outTan                   // z
    );

    [[nodiscard]]
    glm::vec4 RandomColor();

    [[nodiscard]]
    glm::dmat3 SkewSymmetricMatrix(glm::dvec3 const& a);
    
}
