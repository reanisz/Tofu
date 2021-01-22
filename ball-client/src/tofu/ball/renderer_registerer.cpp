#include "tofu/ball/renderer_registerer.h"

#include <Siv3D.hpp>
#include "tofu/ecs/box2d_primitive_renderer.h"

namespace tofu::ball
{
    void RendererRegisterer::OnConstructPlayer(entt::registry& registry, entt::entity entity)
    {
		registry.emplace<Box2DPrimitiveRenderer>(entity, Palette::Aqua, Palette::Black);
    }
    void RendererRegisterer::OnConstructWall(entt::registry& registry, entt::entity entity)
    {
        registry.emplace<Box2DPrimitiveRenderer>(entity, Palette::White, Palette::Black);
    }
    void RendererRegisterer::OnConstructGoal(entt::registry& registry, entt::entity entity)
    {
		registry.emplace<Box2DPrimitiveRenderer>(entity, Color{255, 255, 0, 64}, Color{ 0,0,0,0 });
    }
    void RendererRegisterer::OnConstructGoalFrame(entt::registry& registry, entt::entity entity)
    {
		registry.emplace<Box2DPrimitiveRenderer>(entity, Palette::White, Palette::Black);
    }
}