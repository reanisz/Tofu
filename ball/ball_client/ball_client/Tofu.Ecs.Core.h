#pragma once

// TODO: Siv3Dへの依存はなくす(サーバーではSiv3Dリンクしたくないので)
#include <Siv3D.hpp>
#include <entt/entt.hpp>

namespace tofu
{
	using tVec2 = s3d::Float2;
	using Angle = float;

	struct Transform {
		tVec2 _pos;
		Angle _angle;
	};

}

