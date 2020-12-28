#pragma once

#include <Siv3D.hpp>
#include <entt/entt.hpp>
#include <box2d/box2d.h>

#include "Tofu.Utils.h"
#include "Tofu.Ecs.Core.h"
#include "Tofu.Ecs.Physics.h"

namespace tofu {
	struct Box2DPrimitiveRenderer
	{
		Color _fillColor;
		Color _frameColor;
	};

	class Box2DPrimitiveRenderSystem
	{
	public:
		Box2DPrimitiveRenderSystem(observer_ptr<entt::registry> registry, float scale);

		float GetScale() const;
		void Render();

	private:
		void RenderShape(const Transform& transform, const Box2DPrimitiveRenderer& config, b2CircleShape* shape);
		void RenderShape(const Transform& transform, const Box2DPrimitiveRenderer& config, b2PolygonShape* shape);
		void RenderShape(const Transform& transform, const Box2DPrimitiveRenderer& config, b2EdgeShape* shape);
		void RenderShape(const Transform& transform, const Box2DPrimitiveRenderer& config, b2ChainShape* shape);

		template<class TVec>
		TVec rotate(const TVec& pos, float angle)
		{
			return Mat3x2::Rotate(angle).transform(pos);
		}

	private:
		observer_ptr<entt::registry> _registry;
		float _factor;
	};


}
