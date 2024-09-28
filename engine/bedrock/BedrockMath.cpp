#include "BedrockMath.hpp"

#include "BedrockAssert.hpp"

namespace MFA::Math
{

    //-------------------------------------------------------------------------------------------------

    glm::quat ToQuat(const float xDeg, const float yDeg, const float zDeg)
    {
        return glm::angleAxis(glm::radians(xDeg), Math::RightVec3)
            * glm::angleAxis(glm::radians(yDeg), Math::UpVec3)
            * glm::angleAxis(glm::radians(zDeg), Math::ForwardVec3);
    }

    //-------------------------------------------------------------------------------------------------

    glm::quat ToQuat(glm::vec3 const & eulerAngles)
    {
        return ToQuat(eulerAngles.x, eulerAngles.y, eulerAngles.z);
    }

    //-------------------------------------------------------------------------------------------------

    // Returns degree
    glm::vec3 ToEulerAngles(glm::quat const & quaternion)
    {
        auto angles = glm::degrees(glm::eulerAngles(quaternion));
        if (std::fabs(angles.z) >= 90)
        {
            angles.x += 180.f;
            angles.y = 180.f - angles.y;
            angles.z += 180.f;
        }
        return angles;
    }

    //-------------------------------------------------------------------------------------------------

    #define ROTATE_EULER_ANGLE(transform, angles)                                                  \
    transform = glm::rotate(transform, glm::radians(angles[0]), glm::vec3(1.0f, 0.0f, 0.0f));      \
    transform = glm::rotate(transform, glm::radians(angles[1]), glm::vec3(0.0f, 1.0f, 0.0f));      \
    transform = glm::rotate(transform, glm::radians(angles[2]), glm::vec3(0.0f, 0.0f, 1.0f));      \

    //-------------------------------------------------------------------------------------------------

    void RotateWithEulerAngle(glm::mat4 & inOutTransform, float eulerAngles[3])
    {
        ROTATE_EULER_ANGLE(inOutTransform, eulerAngles);
    }

    //-------------------------------------------------------------------------------------------------

    void RotateWithEulerAngle(glm::mat4 & inOutTransform, glm::vec3 const & eulerAngles)
    {
        ROTATE_EULER_ANGLE(inOutTransform, eulerAngles);
    }

    //-------------------------------------------------------------------------------------------------

    glm::mat4 RotateWithEulerAngle(glm::vec3 const& eulerAngles)
    {
        auto mat4 = glm::identity<glm::mat4>();
        ROTATE_EULER_ANGLE(mat4, eulerAngles);
        return mat4;
    }

    //-------------------------------------------------------------------------------------------------

    #define ROTATE_RADIANS(transform, radians)                                        \
    transform = glm::rotate(transform, radians[0], glm::vec3(1.0f, 0.0f, 0.0f));      \
    transform = glm::rotate(transform, radians[1], glm::vec3(0.0f, 1.0f, 0.0f));      \
    transform = glm::rotate(transform, radians[2], glm::vec3(0.0f, 0.0f, 1.0f));      \

    //-------------------------------------------------------------------------------------------------

    void RotateWithRadians(glm::mat4 & inOutTransform, float radians[3])
    {
        ROTATE_RADIANS(inOutTransform, radians);
    }

    //-------------------------------------------------------------------------------------------------

    void RotateWithRadians(glm::mat4 & inOutTransform, glm::vec3 radians)
    {
        ROTATE_RADIANS(inOutTransform, radians);
    }

    //-------------------------------------------------------------------------------------------------

    void Scale(glm::mat4 & transform, float scale[3])
    {
        transform = glm::scale(
            transform,
            glm::vec3(
                scale[0],
                scale[1],
                scale[2]
            )
        );
    }

    //-------------------------------------------------------------------------------------------------

    void Scale(glm::mat4 & transform, glm::vec3 const & scale)
    {
        transform = glm::scale(
            transform,
            scale
        );
    }

    //-------------------------------------------------------------------------------------------------

    void Scale(glm::mat4& transform, float const scale)
    {
        transform = glm::scale(
            transform,
            glm::vec3 {scale, scale, scale}
        );
    }

	//-------------------------------------------------------------------------------------------------

    glm::mat4 Scale(glm::vec3 const & scale)
    {
        return glm::scale(glm::identity<glm::mat4>(), scale);
    }

    //-------------------------------------------------------------------------------------------------

    glm::mat4 Scale(float scale)
    {
        return Scale(glm::vec3{ scale, scale, scale });
    }

    //-------------------------------------------------------------------------------------------------

    glm::dmat4 Scale(glm::dvec3 const& scale)
    {
        return glm::scale(glm::identity<glm::dmat4>(), scale);
    }

    //-------------------------------------------------------------------------------------------------

    float SignedAngle2d(glm::vec2 const& from, glm::vec2 const& to)
    {
        auto const angle = std::atan2(
            (from.x * to.y) - (to.x * from.y),
            glm::dot(from, to)
        );
        return angle;
    }

    //-------------------------------------------------------------------------------------------------

    float SignedAngle(glm::vec3 const& from, glm::vec3 const& to)
    {
        auto const cross = glm::cross(from, to);
        auto const length = glm::length(cross);
        auto const vn = cross / length;
        auto const top = glm::dot(cross, vn);
        auto const bottom = glm::dot(from, to);
        auto const angle = std::atan2(top, bottom);
        return angle;
    }

    //-------------------------------------------------------------------------------------------------

    glm::mat4 ChangeOfBasis(glm::vec3 const& biTangent, glm::vec3 const& normal, glm::vec3 const& tangent)
    {
	    glm::mat4 changeOfBasisMat{};
	    changeOfBasisMat[0][0] = biTangent.x;
	    changeOfBasisMat[0][1] = biTangent.y;
	    changeOfBasisMat[0][2] = biTangent.z;
	    changeOfBasisMat[1][0] = normal.x;
	    changeOfBasisMat[1][1] = normal.y;
	    changeOfBasisMat[1][2] = normal.z;
	    changeOfBasisMat[2][0] = tangent.x;
	    changeOfBasisMat[2][1] = tangent.y;
	    changeOfBasisMat[2][2] = tangent.z;
	    changeOfBasisMat[3][3] = 1.0f;
        return changeOfBasisMat;
    }

    //-------------------------------------------------------------------------------------------------

    glm::vec4 WorldSpaceToProjectedSpace(glm::vec4 const& worldPosition, glm::mat4 const& viewProjection)
    {
	    auto projectedPos = viewProjection * worldPosition;
	    projectedPos.x /= projectedPos.w;
	    projectedPos.y /= projectedPos.w;
	    projectedPos.z /= projectedPos.w;
	    projectedPos.w = 1.0f;

	    return projectedPos;
    }

    //-------------------------------------------------------------------------------------------------

    glm::vec2 ScreenSpaceToProjectedSpace(glm::vec2 const& point, float const screenWidth, float const screenHeight)
    {
	    glm::vec2 projectedSpace{ point.x, point.y };
	    float const width = screenWidth;
	    float const height = screenHeight;
	    projectedSpace.x = (projectedSpace.x / width) * 2.0f - 1.0f;
	    projectedSpace.y = (projectedSpace.y / height) * 2.0f - 1.0f;

	    return projectedSpace;
    }

    //-------------------------------------------------------------------------------------------------

	// Incorrect
    glm::mat3 TriangleTransform2d(
        glm::vec2 const& localP1, glm::vec2 const& localP2, glm::vec2 const& localP3,
        glm::vec2 const& worldP1, glm::vec2 const& worldP2, glm::vec2 const& worldP3
    )
    {
        auto const lP2P3Diff = localP2 - localP3;
    	auto const lP1P2Diff = localP1 - localP2;

        auto const wP2P3Diff = worldP2 - worldP3;
        auto const wP1P2Diff = worldP1 - worldP2;

        float const s1Top = lP2P3Diff.y * wP1P2Diff.x - lP1P2Diff.y * wP2P3Diff.x;
        float const s1Bottom = lP1P2Diff.x * lP2P3Diff.y - lP2P3Diff.x * lP1P2Diff.y;
        float const s1 = s1Top / s1Bottom;

        auto const s2Top = lP2P3Diff.x * wP1P2Diff.x - lP1P2Diff.x * wP2P3Diff.x;
        auto const s2Bottom = lP1P2Diff.y * lP2P3Diff.x - lP2P3Diff.y * lP1P2Diff.x;
        auto const s2 = s2Top / s2Bottom;

        auto const s3Top = lP2P3Diff.y * wP1P2Diff.y - lP1P2Diff.y * wP2P3Diff.y;
        auto const s3 = s3Top / s1Bottom;

        auto const s4Top = lP2P3Diff.x * wP1P2Diff.y - lP1P2Diff.x * wP2P3Diff.y;
        auto const s4 = s4Top / s2Bottom;

        auto const t1 = worldP1.x - s1 * localP1.x - s2 * localP1.y;
        auto const t2 = worldP1.y - s3 * localP1.x - s4 * localP1.y;

        glm::mat3 matrix {};
        matrix[0][0] = s1;
        matrix[1][0] = s2;
        matrix[2][0] = t1;
        matrix[0][1] = s3;
        matrix[1][1] = s4;
        matrix[2][1] = t2;
        matrix[0][2] = 0;
        matrix[1][2] = 0;
        matrix[2][2] = 1;

        return matrix;
    }

    //-------------------------------------------------------------------------------------------------

    void Translate(glm::mat4 & transform, float distance[3])
    {
        transform = glm::translate(transform, glm::vec3(distance[0], distance[1], distance[2]));
    }

    //-------------------------------------------------------------------------------------------------

    void Translate(glm::mat4 & transform, glm::vec3 const & distance)
    {
        transform = glm::translate(transform, distance);
    }

    //-------------------------------------------------------------------------------------------------

    glm::mat4 Translate(glm::vec3 const& distance)
    {
        return glm::translate(glm::identity<glm::mat4>(), distance);
    }

    //-------------------------------------------------------------------------------------------------

    glm::dmat4 Translate(glm::dvec3 const& distance)
    {
        return glm::translate(glm::identity<glm::dmat4>(), distance);
    }

    //-------------------------------------------------------------------------------------------------

    void PerspectiveProjection(
        float outMatrix[16],
        float const aspectRatio,
        float const fieldOfView,
        float const nearPlane,
        float const farPlane
    )
    {
        float const invTan = 1.0f / tan(glm::radians(0.5f * fieldOfView));
        float const invDepthDiff = 1.0f / (nearPlane - farPlane);

        outMatrix[0] = invTan;
        outMatrix[1 * 4 + 1] = invTan * aspectRatio;
        outMatrix[2 * 4 + 2] = 1.0f * farPlane * invDepthDiff;
        outMatrix[3 * 4 + 2] = 1.0f * nearPlane * farPlane * invDepthDiff;
        outMatrix[2 * 4 + 3] = -1.0f;
    }

    //-------------------------------------------------------------------------------------------------

    void PerspectiveProjection(
        glm::mat4 & outMatrix,
        float const aspectRatio,
        float const fieldOfView,
        float const nearPlane,
        float const farPlane
    )
    {
        float const invTan = 1.0f / tan(glm::radians(0.5f * fieldOfView));
        float const invDepthDiff = 1.0f / (nearPlane - farPlane);

        outMatrix[0][0] = invTan;
        MFA_ASSERT(outMatrix[0][1] == 0.0f);
        MFA_ASSERT(outMatrix[0][2] == 0.0f);
        MFA_ASSERT(outMatrix[0][3] == 0.0f);

        MFA_ASSERT(outMatrix[1][0] == 0.0f);
        outMatrix[1][1] = invTan * aspectRatio;
        MFA_ASSERT(outMatrix[1][2] == 0.0f);
        MFA_ASSERT(outMatrix[1][3] == 0.0f);

        MFA_ASSERT(outMatrix[2][0] == 0.0f);
        MFA_ASSERT(outMatrix[2][1] == 0.0f);
        outMatrix[2][2] = 1.0f * farPlane * invDepthDiff;
        outMatrix[2][3] = -1.0f;

        MFA_ASSERT(outMatrix[3][0] == 0.0f);
        MFA_ASSERT(outMatrix[3][1] == 0.0f);
        outMatrix[3][2] = 1.0f * nearPlane * farPlane * invDepthDiff;
        MFA_ASSERT(outMatrix[3][3] == 0.0f);

    }

    //-------------------------------------------------------------------------------------------------

    void OrthographicProjection(
        glm::mat4 & outMatrix,
        float const leftPlane,
        float const rightPlane,
        float const bottomPlane,
        float const topPlane,
        float const nearPlane,
        float const farPlane
    )
    {
        outMatrix[0][0] = 2.0f / (rightPlane - leftPlane);
        MFA_ASSERT(outMatrix[0][1] == 0.0f);
        MFA_ASSERT(outMatrix[0][2] == 0.0f);
        MFA_ASSERT(outMatrix[0][3] == 0.0f);

        MFA_ASSERT(outMatrix[1][0] == 0.0f);
        outMatrix[1][1] = 2.0f / (bottomPlane - topPlane);
        MFA_ASSERT(outMatrix[1][2] == 0.0f);
        MFA_ASSERT(outMatrix[1][3] == 0.0f);

        MFA_ASSERT(outMatrix[2][0] == 0.0f);
        MFA_ASSERT(outMatrix[2][1] == 0.0f);
        outMatrix[2][2] = 1.0f / (nearPlane - farPlane);
        MFA_ASSERT(outMatrix[2][3] == 0.0f);

        outMatrix[3][0] = -(rightPlane + leftPlane) / (rightPlane - leftPlane);
        outMatrix[3][1] = -(bottomPlane + topPlane) / (bottomPlane - topPlane);
        outMatrix[3][2] = nearPlane / (nearPlane - farPlane);
        outMatrix[3][3] = 1.0f;
    }

    //-------------------------------------------------------------------------------------------------

    float UnSignedAngle(glm::vec3 const& a, glm::vec3 const& b)
    {
        auto const dot = glm::dot(glm::normalize(a), glm::normalize(b));
        if (IsEqualUsingDot(dot))
        {
            return 0.0f;
        }
        return ACosSafe(dot);
    }

    //-------------------------------------------------------------------------------------------------

    float UnSignedAngle(glm::quat const& a, glm::quat const& b)
    {
        auto const dot = glm::dot(a, b);
        if (IsEqualUsingDot(dot))
        {
            return 0.0f;
        }
        return ACosSafe(dot) * 2.0f;
    }

    //-------------------------------------------------------------------------------------------------

    bool IsEqualUsingDot(float const dot)
    {
        // Returns false in the presence of NaN values.
        return dot >= 1.0f - glm::epsilon<float>();
    }

    //-------------------------------------------------------------------------------------------------

    bool IsEqual(glm::quat const& a, glm::quat const& b)
    {
        auto const dot = glm::dot(a, b);
        return IsEqualUsingDot(dot);
    }

    //-------------------------------------------------------------------------------------------------

    glm::quat FindRotation(glm::vec3 const& from, glm::vec3 const& to)
    {
        auto const theta = Math::ACosSafe(glm::dot(glm::normalize(to), glm::normalize(from)));
        if (IsNearZero(theta) == true)
        {
            return glm::identity<glm::quat>();
        }

        auto cross = glm::cross(from, to);
        auto const length = glm::length(cross);
        if (length <= glm::epsilon<float>())
        {
            return glm::identity<glm::quat>();
        }
        cross = cross / length;
        auto const rotation = glm::angleAxis(theta, cross);

        MFA_ASSERT(std::isnan(rotation.x) == false);
        MFA_ASSERT(std::isnan(rotation.y) == false);
        MFA_ASSERT(std::isnan(rotation.z) == false);
        MFA_ASSERT(std::isnan(rotation.w) == false);

        return rotation;
    }

    //-------------------------------------------------------------------------------------------------

    glm::vec3 MoveTowards(glm::vec3 const& from, glm::vec3 const& to, float const maxDistance)
    {
        auto const vector = to - from;
        auto const distance = glm::length(vector);
        if (distance < glm::epsilon<float>())
        {
            return to;
        }
        float const t = std::min(maxDistance/distance, 1.0f);
        return glm::mix(from, to, t);
    }

    //-------------------------------------------------------------------------------------------------

    glm::quat RotateTowards(glm::quat const& from, glm::quat const& to, float const maxDegreeDelta)
    {
        MFA_ASSERT(maxDegreeDelta > 0.0f);

        auto cosTheta = glm::dot(from, to);
        if (IsEqualUsingDot(cosTheta))
        {
            return to;
        }

        glm::quat to2 = to;
        
        // If cosTheta < 0, the interpolation will take the long way around the sphere.
        // To fix this, one quat must be negated.
        if (cosTheta < 0.0f)
        {
            to2 = -to;
            cosTheta = -cosTheta;
        }

        float const halfTheta = Math::ACosSafe(cosTheta);
        auto const theta = halfTheta * 2.0f;

        auto const fraction = std::clamp(maxDegreeDelta / theta, 0.0f, 1.0f);

        // Perform a linear interpolation when cosTheta is close to 1 to avoid side effect of sin(angle) becoming a zero denominator
        if (cosTheta > 1.0f - glm::epsilon<float>())
        {
            // Linear interpolation
            return glm::quat(
                glm::mix(from.w, to2.w, fraction),
                glm::mix(from.x, to2.x, fraction),
                glm::mix(from.y, to2.y, fraction),
                glm::mix(from.z, to2.z, fraction)
            );
        }
        else
        {
            // Essential Mathematics, page 467
            return (sinf((1.0f - fraction) * halfTheta) * from + sinf(fraction * halfTheta) * to2) / sinf(halfTheta);
        }
    }
    
    //-------------------------------------------------------------------------------------------------

    bool IsValid(float value)
    {
        if (std::isnan(value) || std::isinf(value))
        {
            return false;
        }
        return true;
    }

    //-------------------------------------------------------------------------------------------------

    bool IsValid(double value)
    {
        if (std::isnan((float)value) || std::isinf((float)value))
        {
            return false;
        }
        return true;
    }

    //-------------------------------------------------------------------------------------------------

    bool IsValid(glm::dvec3 const & vec)
    {
        if (IsValid(vec.x) == false ||
            IsValid(vec.y) == false ||
            IsValid(vec.z) == false)
        {
            return false;
        }

        return true;
    }

    //-------------------------------------------------------------------------------------------------

    bool IsValid(glm::vec3 const & vec)
    {
        if (IsValid(vec.x) == false ||
            IsValid(vec.y) == false ||
            IsValid(vec.z) == false)
        {
            return false;
        }

        return true;
    }

    //-------------------------------------------------------------------------------------------------

    double CalculateVolume(
        glm::dvec3 const& p0,
        glm::dvec3 const& p1,
        glm::dvec3 const& p2,
        glm::dvec3 const& p3
    )
    {
        auto const diff1 = p1 - p0;
        auto const diff2 = p2 - p0;
        auto const diff3 = p3 - p0;

        auto const F00 = diff1.x;
        auto const F01 = diff1.y;
        auto const F02 = diff1.z;

        auto const F10 = diff2.x;
        auto const F11 = diff2.y;
        auto const F12 = diff2.z;

        auto const F20 = diff3.x;
        auto const F21 = diff3.y;
        auto const F22 = diff3.z;

        auto const H = F11 * F22 - F21 * F12;
        auto const I = F21 * F02 - F22 * F01;
        auto const J = F01 * F12 - F11 * F02;

        auto const L = F00 * H;
        auto const M = F10 * I;
        auto const N = F20 * J;
        auto const detF = L + M + N;

        return detF;
    }

    //-------------------------------------------------------------------------------------------------

    std::vector<std::tuple<int, int>> Rasterize(int xs, int ys, int xe, int ye)
    {
        if (ys > ye)
        {
            auto const temp = ys;
            ys = ye;
            ye = temp;
        }

	    int const yDiff = ye - ys;
	    int const xDiff = xe - xs;

	    int xi = xs;
	    int xf = -xDiff;
	    int const mi = yDiff / xDiff;
	    int const mf = 2 * (yDiff % xDiff);

	    int y = ys;

        std::vector<std::tuple<int, int>> result{};

	    while (y != ye)
	    {
            result.emplace_back(std::tuple{ xi, y });

            ++y;

		    xi += mi;
		    xf += mf;
		    if (xf > 0)
		    {
			    xi += 1;
			    xf = xf - 2 * yDiff;
		    }
	    }

        return result;
    }

    //-------------------------------------------------------------------------------------------------

    std::vector<std::tuple<int, int, int>> Rasterize(
        int xs, 
        int ys, 
        int zs, 
        int xe, 
        int ye,
	    int ze
    )
    {
        if (ys > ye)
        {
            auto const temp = ys;
            ys = ye;
            ye = temp;
        }

        int const yDiff = ye - ys;
        int const xDiff = xe - xs;
        int const zDiff = ze - zs;

        int xi = xs;
        int xf = -xDiff;
        int const x_mi = yDiff / xDiff;
        int const x_mf = 2 * (yDiff % xDiff);

        int zi = zs;
        int zf = -zDiff;
        int const z_mi = yDiff / zDiff;
        int const z_mf = 2 * (yDiff % zDiff);

        int y = ys;

        std::vector<std::tuple<int, int, int>> result{};

        while (y != ye)
        {
            result.emplace_back(std::tuple{ xi, y, zi });

            ++y;

            xi += x_mi;
            xf += x_mf;
            if (xf > 0)
            {
                xi += 1;
                xf = xf - 2 * yDiff;
            }

            zi += z_mi;
            zf += z_mf;
            if (zf > 0)
            {
                zi += 1;
                zf = zf - 2 * zDiff;
            }
        }

        return result;
    }

    //-------------------------------------------------------------------------------------------------

	glm::dvec3 ToLocalCoordinate(
        glm::dvec3 const& input,

        glm::dvec3 const& center,
        glm::dvec3 const& biTan,
	    glm::dvec3 const& normal,
        glm::dvec3 const& tan
    )
    {
        return glm::dvec3 {
            glm::dot(input - center, biTan),
            glm::dot(input - center, normal),
            glm::dot(input - center, tan)
        };
    }

    //-------------------------------------------------------------------------------------------------

	void GenerateLocalCoordinate(
        glm::dvec3 const& p0, 
        glm::dvec3 const& p1, 
        glm::dvec3 const& p2,

        glm::dvec3 & outCenter,
        glm::dvec3 & outBiTan, 
        glm::dvec3 & outNormal, 
        glm::dvec3 & outTan
    )
	{
        outCenter = (p0 + p1 + p2) / 3.0;
    	outNormal = glm::normalize(glm::cross(p1 - p0, p2 - p1));
        outBiTan = glm::normalize(p1 - p0);
        outTan = glm::normalize(glm::cross(outBiTan, outNormal));
	}

    //-------------------------------------------------------------------------------------------------

	glm::vec4 RandomColor()
	{
		return glm::vec4{ Random(0.0, 1.0), Random(0.0, 1.0), Random(0.0, 1.0), 1.0 };
	}

    //-------------------------------------------------------------------------------------------------

	glm::dmat3 SkewSymmetricMatrix(glm::dvec3 const& a)
	{
        return glm::dmat3(
            0.0f, -a.z, a.y,
            a.z, 0.0f, -a.x,
            -a.y, a.x, 0.0f
        );
	}

	//-------------------------------------------------------------------------------------------------

}
