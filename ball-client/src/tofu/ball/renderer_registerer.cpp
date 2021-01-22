#include "tofu/ball/renderer_registerer.h"

#include <Siv3D.hpp>
#include <tofu/utils.h>

#include "tofu/ecs/box2d_primitive_renderer.h"
#include "tofu/ball/game.h"

#include "tofu/ball/player.h"
#include "tofu/ball/stage.h"

#include "tofu/renderer/siv3d.h"
#include "tofu/ball/input.h"
#include "tofu/ball/frame_updater.h"

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

        auto renderer = service_locator->Register(std::make_unique<S3DRenderSystem>());
        auto box2d_primitive_renderer = service_locator->Register(std::make_unique<Box2DPrimitiveRenderSystem>(service_locator, registry, 100));

        auto camera = service_locator->Register(std::make_unique<s3d::Camera2D>());
        camera->setCenter({ 400,300 });

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

        {
            using namespace tofu::jobs;
            using namespace tofu::ball::jobs;

            auto job_scheduler = service_locator->Get<JobScheduler>();
            job_scheduler->Register(make_job<S3DStartRender>({ get_job_tag<EndUpdate>() }, renderer));

            job_scheduler->Register(make_job<RenderBox2DPrimitives>({ get_job_tag<S3DStartRender>() }, box2d_primitive_renderer));
            job_scheduler->Register(make_job<RenderMousePosition>({ get_job_tag<S3DStartRender>() }, service_locator->Get<InputSystem>(), renderer));

            job_scheduler->Register(make_job<S3DEndRender>({ get_job_tag<RenderBox2DPrimitives>(), get_job_tag<RenderMousePosition>() }, renderer));
        }
    }
}