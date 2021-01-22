#include "tofu/ball/renderer_registerer.h"

#include <Siv3D.hpp>
#include "tofu/ecs/box2d_primitive_renderer.h"
#include "tofu/ball/game.h"

#include "tofu/ball/player.h"
#include "tofu/ball/stage.h"

namespace tofu::ball
{
    void RendererRegisterer::OnConstructPlayer(entt::registry& registry, entt::entity entity)
    {
		registry.emplace<Box2DPrimitiveRenderer>(entity, Palette::Aqua, Palette::Black);
    }
    void RendererRegisterer::OnConstructBall(entt::registry& registry, entt::entity entity)
    {
        registry.emplace<Box2DPrimitiveRenderer>(entity, Palette::White, Palette::Black);
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

    void RendererRegisterer::Attach(Game& game)
    {
        auto service_locator = game.getServiceLocator();
        auto registry = game.getRegistry();

        auto renderer_registerer = service_locator->Register(std::make_unique<RendererRegisterer>());

        registry->on_construct<Player>()
            .connect<&RendererRegisterer::OnConstructPlayer>(renderer_registerer.get());
        registry->on_construct<Ball>()
            .connect<&RendererRegisterer::OnConstructBall>(renderer_registerer.get());
        registry->on_construct<Goal>()
            .connect<&RendererRegisterer::OnConstructGoal>(renderer_registerer.get());
        registry->on_construct<GoalFrame>()
            .connect<&RendererRegisterer::OnConstructGoalFrame>(renderer_registerer.get());
        registry->on_construct<Wall>()
            .connect<&RendererRegisterer::OnConstructWall>(renderer_registerer.get());
    }
}