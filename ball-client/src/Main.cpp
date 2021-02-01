#include <Siv3d.hpp>

#include <fmt/core.h>

#include "tofu/ball/game.h"
#include "tofu/ball/renderer_registerer.h"

#include "tofu/ball/frame_updater.h"
#include "tofu/ball/input.h"
#include "tofu/ball/player_controller.h"
#include "tofu/ball/actions.h"

#include "tofu/ball/net_client.h"

#undef GetJob

void not_called_function()
{
    clearerr(nullptr);
}

namespace tofu::ball
{
    void init_as_client(Game& game)
    {
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

            job_scheduler->Register(make_job<StepInput>({ get_job_tag<StartFrame>() }, {}, input_system));
            job_scheduler->Register(make_job<ProcessPlayerControl>({ get_job_tag<StepInput>() }, {}, player_controller));
            job_scheduler->GetJob(get_job_tag<StepAction>())->AddDependency(get_job_tag<ProcessPlayerControl>());
        }

        // クライアントなので描画を有効化
        tofu::ball::RendererRegisterer::Attach(game);
    }

    void update_as_client(Game& game)
    {
        auto service_locator = game.getServiceLocator();
        service_locator->Get<tofu::ball::InputSystem>()->Update();
        game.update();

        auto transform = service_locator->Get<Camera2D>()->createTransformer();

        if (service_locator->Get<tofu::S3DRenderSystem>()->HasData()) {
            service_locator->Get<tofu::S3DRenderSystem>()->Render();
        }
    }

    class Siv3DClient : public Client
    {
    public:
        using Client::Client;
    protected:

        virtual void InitGame() override
        {
            _game.initBaseSystems();

            init_as_client(_game);

            _game.initEnitites();
        }

        virtual void UpdateGame() override
        {
            update_as_client(_game);
        }
    };

    void run_as_client()
    {
        Siv3DClient client(Siv3DClient::Config{});
        client.Run();

        while (client.GetState() == Client::State::Lobby && System::Update())
        {
            client.UpdateAtLobby();
        }

        while (client.GetState() == Client::State::InGame && System::Update())
        {
            client.UpdateIngame();
        }
    }

    void run_as_standalone()
    {
        Game _game;
        _game.initBaseSystems();

        init_as_client(_game);

        _game.initEnitites();

        _game.start();

        while (System::Update())
        {
            update_as_client(_game);
        }
    }
}

void Main()
{
    Scene::SetBackground(ColorF(0.8, 0.9, 1.0));
    tofu::ball::run_as_standalone();
    return;
}

