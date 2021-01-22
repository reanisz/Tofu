#pragma once

#include <entt/entt.hpp>

namespace tofu::ball
{
    class Game;

    class RendererRegisterer
    {
    public:
        void OnConstructPlayer(entt::registry& registry, entt::entity entity);
        void OnConstructBall(entt::registry& registry, entt::entity entity);

        void OnConstructWall(entt::registry& registry, entt::entity entity);
        void OnConstructGoal(entt::registry& registry, entt::entity entity);
        void OnConstructGoalFrame(entt::registry& registry, entt::entity entity);

        static void Attach(Game& game);
    };
}
