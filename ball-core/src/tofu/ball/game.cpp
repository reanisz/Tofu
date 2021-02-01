#include "tofu/ball/game.h"

#include <tofu/utils/job.h>
#include <tofu/ecs/core.h>
#include <tofu/ecs/physics.h>

#include "tofu/ball/actions.h"
#include "tofu/ball/stage.h"
#include "tofu/ball/player.h"
#include "tofu/ball/frame_updater.h"

#include "tofu/ball/network.h"
#include "tofu/ball/sync.h"

namespace tofu::ball 
{
    Game::Game()
    {
    }
    void Game::initBaseSystems()
    {
        initSystems();
    }
    void Game::initEnitites()
    {
        initStage();
        initBall();
        initPlayers();
    }
    void Game::start()
    {
        _serviceLocator.Get<UpdateSystem>()->Start();
    }
    void Game::update()
    {
    }
    observer_ptr<entt::registry> Game::getRegistry()
    {
        return &_registry;
    }
    observer_ptr<ServiceLocator> Game::getServiceLocator()
    {
        return &_serviceLocator;
    }

    void Game::initSystems()
    {
        // === Core ===
        _serviceLocator.Register(std::make_unique<TickCounter>());

        // === Physics ===
        auto physics = _serviceLocator.Register(std::make_unique<Physics>(&_registry));

        // === Simulation ===
        auto update_system = _serviceLocator.Register(std::make_unique<UpdateSystem>(&_serviceLocator, &_registry));

        _serviceLocator.Register(std::make_unique<ActionQueue>());
        auto action_system = _serviceLocator.Register(std::make_unique<ActionSystem>(&_serviceLocator, &_registry));

        // === Net ===
        auto sync_system = _serviceLocator.Register(std::make_unique<CompletelySyncSystem>(&_serviceLocator, &_registry, MaxPlayerNum, ActionDelay));

        // === Job ===
        auto job_scheduler = _serviceLocator.Register(std::make_unique<JobScheduler>());
        {
            using namespace jobs;
            using namespace tofu::jobs;
            using namespace tofu::ball::jobs;
            using namespace tofu::ball::job_conditions;

            job_scheduler->Register(make_job<StartFrame>({}, {}, update_system));
            job_scheduler->Register(make_job<CheckStepable>({ get_job_tag<StartFrame>() }, {}, sync_system));
            job_scheduler->Register(make_job<ApplySyncBufferToActionQueue>({ get_job_tag<CheckStepable>() }, { get_condition_tag<IsStepable>() }, sync_system));
            job_scheduler->Register(make_job<StepAction>({ get_job_tag<ApplySyncBufferToActionQueue>() }, { get_condition_tag<IsStepable>() }, action_system));
            job_scheduler->Register(make_job<StepPhysics>({ get_job_tag<StepAction>() }, { get_condition_tag<IsStepable>() }, physics));

            job_scheduler->Register(make_job<EndUpdate>({ get_job_tag<StepAction>(), get_job_tag<StepPhysics>() }, {}));
            job_scheduler->Register(make_job<StepSyncBuffer>({ get_job_tag<EndUpdate>() }, { get_condition_tag<IsStepable>() }, sync_system));
        }
    }
    void Game::initStage()
    {
        generate_stage(&_serviceLocator, &_registry);
    }
    void Game::initPlayers()
    {
        auto physics = _serviceLocator.Get<Physics>();
        {
            // Player 
            Player::Generate(&_serviceLocator, &_registry, 0, {1.0f, 5.0f});
            Player::Generate(&_serviceLocator, &_registry, 1, {7.0f, 5.0f});
        }
    }
    void Game::initBall()
    {
        auto physics = _serviceLocator.Get<Physics>();
        {
            // ball
            auto entity = _registry.create();
            _registry.emplace<Transform>(entity, tVec2{ 4.f, 0.5f }, 0.0f);
            auto ball = _registry.emplace<Ball>(entity, 0.28f);

            b2BodyDef body_def;
            body_def.type = b2BodyType::b2_dynamicBody;

            auto body = physics->GenerateBody(entity, body_def)._body;
            b2CircleShape shape;
            shape.m_radius = ball._radius;
            b2FixtureDef fixture_def;
            fixture_def.shape = &shape;
            fixture_def.density = 0.22f;
            fixture_def.friction = 0.9f;
            fixture_def.restitution = 0.85f;

            body->CreateFixture(&fixture_def);
        }
    }
}
