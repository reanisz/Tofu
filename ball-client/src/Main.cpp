#include <Siv3d.hpp>

#include <fmt/core.h>

#include "tofu/ball/game.h"
#include "tofu/ball/renderer_registerer.h"

#include "tofu/ball/frame_updater.h"
#include "tofu/ball/input.h"
#include "tofu/ball/player_controller.h"
#include "tofu/ball/actions.h"

#include "tofu/ball/net_client.h"
#include "tofu/ball/sync.h"

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

            job_scheduler->Register(make_job<StepInput>({ get_job_tag<StepTick>() }, { get_condition_tag<job_conditions::IsStepable>() }, input_system));
            job_scheduler->Register(make_job<ProcessPlayerControl>({ get_job_tag<StepInput>() }, { get_condition_tag<job_conditions::IsStepable>() }, player_controller));
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

            {
                auto registry = _game.getRegistry();
                auto service_locator = _game.getServiceLocator();

                auto job_scheduler = service_locator->Get<JobScheduler>();
                auto quic = service_locator->Register(std::make_unique<QuicControllerSystem>(service_locator, registry));

                using namespace tofu::jobs;
                using namespace tofu::ball::jobs;
                job_scheduler->Register(make_job<ApplySyncObject>({ get_job_tag<StartFrame>() }, {}, quic));
                job_scheduler->GetJob(get_job_tag<StepTick>())->AddDependency(get_job_tag<ApplySyncObject>());
            }

            _game.initEnitites();
        }

        virtual void UpdateGame() override
        {
            update_as_client(_game);
        }
    };

    void run_as_client(const std::string& ip)
    {
        Siv3DClient::Config config =
        {
            ._ip = ip,
        };
        Siv3DClient client(config);
        client.Run();

        for (int i = 0; i < 100; i++)
        {
            if (!System::Update())
                return;
        }

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

void Menu()
{
    TextEditState textedit_ip{ U"127.0.0.1" };
    const Font font(24);

    while (System::Update())
    {
        auto pos = Point{ 100, 100 };
        auto font_area = font(U"IP Address:").draw(s3d::Arg::topLeft = pos, Palette::Black);
        if (SimpleGUI::TextBox(textedit_ip, pos + Point{ static_cast<int>(font_area.w), 0 }))
        {
        }
        pos.y += font_area.h;
        if (SimpleGUI::Button(U"Connect", pos))
        {
            tofu::ball::run_as_client(textedit_ip.text.toUTF8().c_str());
            return;
        }
    }
}

void Main()
{
    Scene::SetBackground(ColorF(0.8, 0.9, 1.0));
    Menu();
    return;
}

