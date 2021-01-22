#pragma once

#include <entt/entt.hpp>
#include "tofu/utils.h"


namespace tofu
{
	using Angle = float;

	struct Transform {
		tVec2 _pos;
		Angle _angle;
	};
}

