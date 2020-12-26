#pragma once

// TODO: Siv3D�ւ̈ˑ��͂Ȃ���(�T�[�o�[�ł�Siv3D�����N�������Ȃ��̂�)
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

