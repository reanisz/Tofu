#include "tofu/ball/player.h"

#include <tofu/ecs/core.h>
#include <tofu/ecs/physics.h>

namespace tofu::ball
{
    std::tuple<entt::entity, Player&> Player::Generate(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry, int id, tVec2 pos)
    {
        auto physics = service_locator->Get<Physics>();

        auto entity = registry->create();
        registry->emplace<Transform>(entity, pos, 0.0f);

        auto player = registry->emplace<Player>(entity);
        player._id = id;

        b2BodyDef body_def;
        body_def.type = b2BodyType::b2_dynamicBody;

        auto body = physics->GenerateBody(entity, body_def)._body;
        b2CircleShape shape;
        shape.m_radius = 0.14f;
        b2FixtureDef fixture_def;
        fixture_def.shape = &shape;
        fixture_def.density = 1.0f;
        fixture_def.friction = 0.9f;
        fixture_def.restitution = 0.4f;

        body->CreateFixture(&fixture_def);

        return { entity, player };
    }

}