#pragma once

#include <variant>
#include <cstdint>
#include <thread>
#include <condition_variable>
#include <optional>

#include <entt/entt.hpp>

#include <tofu/utils.h>
#include <tofu/containers.h>
#include <tofu/input.h>

#include <tofu/ecs/core.h>

namespace tofu::ball
{
    inline constexpr std::uint32_t ActionDelay = 4;

    namespace actions 
    {
        struct Null
        {
        };
        struct Move
        {
            tVec2 _target;
        };
        struct Dash 
        {
            tVec2 _target;
        };
        using Variant = std::variant<Null, Move, Dash>;
    }

    struct ActionCommand 
    {
        entt::entity _entity;
        actions::Variant _action;
        GameTick _tick;
    };

    class ActionQueue
    {
    public:
        static constexpr int QueueCount = 4;

        void SetCurrentTick(GameTick tick) noexcept
        {
            _current = tick;
        }

        void Enqueue(ActionCommand&& command) 
        {
            auto tick = command._tick;
            auto d = tick - _current;
            if (d < _queues.size()) {
                _queues[*d].push_back(std::move(command));
            }
            else {
                _futureActions.push_back(command);
            }
        }

        std::vector<ActionCommand> Retrieve() 
        {
            std::vector<ActionCommand> ret;
            std::swap(ret, _queues[0]);

            for (int i = 0; i < _queues.size() - 1; i++) {
                std::swap(_queues[i], _queues[i + 1]);
            }

            auto& left = _queues[_queues.size() - 1];
            auto t = *_current + QueueCount - 1;
            for (auto& act : _futureActions) {
                if (act._tick == t)
                    left.push_back(act);
            }
            std::erase_if(_futureActions, [t](const ActionCommand& v) { return v._tick == t; });

            // NRVO(Named Return Value Optimization)を期待
            return ret;
        }

    private:
        GameTick _current;

        // _queues[x] : x Tick先に処理するAction
        std::array<std::vector<ActionCommand>, QueueCount> _queues;

        // queuesに溢れるくらい未来のaction
        std::vector<ActionCommand> _futureActions;
    };


    class ActionSystem
    {
    public:
        ActionSystem(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry);
        void Step();

    private:
        void apply(entt::entity entity, const actions::Null& action) const noexcept
        {
        }
        void apply(entt::entity entity, const actions::Move& action);
        void apply(entt::entity entity, const actions::Dash& action);

        observer_ptr<ServiceLocator> _serviceLocator;
        observer_ptr<entt::registry> _registry;
    };

    namespace jobs
    {
        class StepAction
        {
        public:
            StepAction(observer_ptr<ActionSystem> system)
                : _system(system)
            {
            }

            void operator()() const
            {
                _system->Step();
            }

        private:
            observer_ptr<ActionSystem> _system;
        };
    }
}

