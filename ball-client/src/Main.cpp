#include "tofu/ball/game.h"
#include "tofu/ball/renderer_registerer.h"

#include "tofu/ball/frame_updater.h"
#include "tofu/ball/input.h"
#include "tofu/ball/player_controller.h"
#include "tofu/ball/actions.h"

void Main()
{
    Scene::SetBackground(ColorF(0.8, 0.9, 1.0));

	tofu::ball::Game game;
    {
        using namespace tofu;
        using namespace tofu::ball;

        game.initBaseSystems();

        auto registry = game.getRegistry();
        auto service_locator = game.getServiceLocator();

        auto job_scheduler = service_locator->Get<JobScheduler>();

        // inputを有効化
        {
            using namespace tofu::jobs;
            using namespace tofu::ball::jobs;
            // === Input ===
            auto input_system = service_locator->Register(std::make_unique<InputSystem>());
            auto player_controller = service_locator->Register(std::make_unique<PlayerController>(service_locator, registry));

            job_scheduler->Register(make_job<StepInput>({get_job_tag<StartFrame>()}, input_system));
            job_scheduler->Register(make_job<ProcessPlayerControl>({get_job_tag<StepInput>()}, player_controller));
            job_scheduler->GetJob(get_job_tag<StepAction>())->AddDependency(get_job_tag<ProcessPlayerControl>());
        }

        // クライアントなので描画を有効化
        tofu::ball::RendererRegisterer::Attach(game);
    }

    game.initEnitites();
	game.run();
}

