#pragma once

#include <box2d/box2d.h>

#include <tofu/utils.h>
#include <tofu/ecs/core.h>

namespace tofu
{
    struct RigidBody {
        b2Body* _body;
    };

    class Physics {
    public:
        Physics(observer_ptr<entt::registry> registry);

        // Transformにbox2d世界を合わせる
        void FollowTransform();

        void Step(float time_step);

        // box2d世界にTransformを合わせる
        void WriteBackToTransform();

        RigidBody& GenerateBody(entt::entity entity);
        RigidBody& GenerateBody(entt::entity entity, const b2BodyDef& body_def);

    private:
        observer_ptr<entt::registry> _registry;
        std::unique_ptr<b2World> _world;
    };

    namespace jobs
    {
        class StepPhysics
        {
        public:
            StepPhysics(observer_ptr<Physics> system)
                : _system(system)
            {
            }

            void operator()() const
            {
                _system->FollowTransform();
                _system->Step(1.f / 60);
                _system->WriteBackToTransform();
            }

        private:
            observer_ptr<Physics> _system;
        };
    }

}
