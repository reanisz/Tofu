#pragma once

#include <entt/entt.hpp>

#include <tofu/utils.h>
#include <tofu/containers.h>
#include <tofu/input.h>

#include <tofu/ecs/core.h>

namespace tofu::ball
{
    class UpdateSystem
    {
    public:
        UpdateSystem(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry);

        void Start();
    
        void StartFrame();

    private:
        void Step();

        observer_ptr<ServiceLocator> _serviceLocator;
        observer_ptr<entt::registry> _registry;

        ScheduledUpdateThread _thread;
    };

    namespace jobs
    {
        class StartFrame
        {
        public:
            StartFrame(observer_ptr<UpdateSystem> system)
                : _system(system)
            {
            }

            void operator()() const
            {
                _system->StartFrame();
            }
        private:
            observer_ptr<UpdateSystem> _system;
        };

        class EndUpdate
        {
        public:
            void operator()() const {
            }
        };
    }
}
