#include "Tofu.Ecs.Box2DPrimitiveRenderer.h"

namespace tofu {
	Box2DPrimitiveRenderSystem::Box2DPrimitiveRenderSystem(observer_ptr<entt::registry> registry, float scale)
		: _registry(registry)
		, _factor(scale)
	{
	}
	float Box2DPrimitiveRenderSystem::GetScale() const
	{
		return _factor;
	}
	void Box2DPrimitiveRenderSystem::Render()
	{
		auto view = _registry->view<Transform, RigidBody, Box2DPrimitiveRenderer>();
		for (auto&& [entity, transform, rigid_body, config] : view.proxy()) {
			auto body = rigid_body._body;
			auto fixture = body->GetFixtureList();
			do {
				auto shape = fixture->GetShape();
				switch (shape->GetType())
				{
				case b2Shape::e_circle:
					RenderShape(transform, config, dynamic_cast<b2CircleShape*>(shape));
					break;
				case b2Shape::e_polygon:
					RenderShape(transform, config, dynamic_cast<b2PolygonShape*>(shape));
					break;
				case b2Shape::e_edge:
					RenderShape(transform, config, dynamic_cast<b2EdgeShape*>(shape));
					break;
				case b2Shape::e_chain:
					RenderShape(transform, config, dynamic_cast<b2ChainShape*>(shape));
					break;
				}
			} while (fixture = fixture->GetNext());
		}
	}
	void Box2DPrimitiveRenderSystem::RenderShape(const Transform& transform, const Box2DPrimitiveRenderer& config, b2CircleShape* shape)
	{
		auto center = _factor * (transform._pos + rotate(Float2{ shape->m_p.x, shape->m_p.y }, transform._angle));
		Circle{ center, _factor * shape->m_radius }
			.draw(config._fillColor)
			.drawFrame(1, config._frameColor);
		auto p2 = center + rotate(Float2{ _factor * shape->m_radius, 0 }, transform._angle);
		Line{ center, p2 }.draw(config._frameColor);
	}
	void Box2DPrimitiveRenderSystem::RenderShape(const Transform& transform, const Box2DPrimitiveRenderer& config, b2PolygonShape* shape)
	{
		static_vector<Vec2, 8> v;
		float angle = transform._angle;

		auto p_shape = dynamic_cast<b2PolygonShape*>(shape);
		for (int i = 0; i < p_shape->m_count; i++) {
			auto pos = p_shape->m_vertices[i];
			v.push_back(_factor * (transform._pos + rotate(Vec2{ pos.x, pos.y }, angle)));
		}

		Polygon polygon{ v.data(), v.size() };
		polygon.draw(config._fillColor).drawFrame(1, config._frameColor);
	}
	void Box2DPrimitiveRenderSystem::RenderShape(const Transform& transform, const Box2DPrimitiveRenderer& config, b2EdgeShape* shape)
	{
		auto p1 = _factor * (transform._pos + rotate(Float2{ shape->m_vertex1.x, shape->m_vertex1.y }, transform._angle));
		auto p2 = _factor * (transform._pos + rotate(Float2{ shape->m_vertex2.x, shape->m_vertex2.y }, transform._angle));

		Line{ p1, p2 }.draw(config._frameColor);
	}
	void Box2DPrimitiveRenderSystem::RenderShape(const Transform& transform, const Box2DPrimitiveRenderer& config, b2ChainShape* shape)
	{
		b2EdgeShape edge;
		for (int i = 0; i < shape->GetChildCount(); i++)
		{
			shape->GetChildEdge(&edge, i);
			RenderShape(transform, config, &edge);
		}
	}
}