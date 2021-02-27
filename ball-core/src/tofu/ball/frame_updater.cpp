#include "tofu/ball/frame_updater.h"

#include "tofu/ball/actions.h"

namespace tofu::ball
{
    UpdateSystem::UpdateSystem(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry)
        : _serviceLocator(service_locator)
        , _registry(registry)
        , _thread(std::chrono::microseconds{ 16'666 }, [this](ScheduledUpdateThread&) { this->Step(); })
    {
    }

    void UpdateSystem::Start()
    {
        _thread.Start();
    }

    void UpdateSystem::StartFrame()
    {
    }

    void UpdateSystem::StepTick()
    {
        _serviceLocator->Get<TickCounter>()->Step();

        auto tick = _serviceLocator->Get<TickCounter>()->GetCurrent();
        _serviceLocator->Get<ActionQueue>()->SetCurrentTick(tick);
    }

    void UpdateSystem::Step()
    {
        _serviceLocator->Get<JobScheduler>()->Run();
    }

}
