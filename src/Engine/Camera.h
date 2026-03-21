#pragma once

#include <Math/rbVec3.h>

#include <Math/rbAngle.h>

struct Camera
{
	rbmk::Math::Vec3 position{ 0, 0, 0 };

	rbmk::Math::Radian yaw{ rbmk::Math::Angle(180) };
	rbmk::Math::Radian pitch{ 0 };

	rbmk::Math::Radian fov{ rbmk::Math::Angle(90) };
};
