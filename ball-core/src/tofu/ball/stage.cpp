#include "tofu/ball/stage.h"

#include <tofu/containers.h>
#include <tofu/ecs/core.h>
#include <tofu/ecs/physics.h>

namespace
{
    float to_radians(float degree)
    {
        return degree * b2_pi / 180;
    }

    template<class T>
    void rotate(T& vec, float degree) {
        for (b2Vec2& v : vec) {
            v = b2Mul(b2Rot{ to_radians(degree) }, v);
        }
    }

    template<class T>
    void translate(T& vec, b2Vec2 offset)
    {
        for (b2Vec2& v : vec) {
            v += offset;
        }
    }
}

namespace tofu::ball
{
    std::tuple<entt::entity> GoalFrame::Generate(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry, tVec2 pos)
    {
        auto entity = registry->create();
        registry->emplace<Transform>(entity, pos, 0.0f);
        auto physics = service_locator->Get<Physics>();

        registry->emplace<GoalFrame>(entity);
        
        b2BodyDef body_def;
        body_def.type = b2BodyType::b2_kinematicBody;

        auto body = physics->GenerateBody(entity, body_def)._body;

        const float w = 0.5f;
        const float h = 0.05f;

        for (int i = 0; i < 2; i++) {
            b2PolygonShape shape;
            stack_vector<b2Vec2, 8> v =
            {
                {-w, -h},
                {+w, -h},
                {+w, +h},
                {-w, +h},
            };
            int k = (i == 0 ? 1 : -1);
            auto offset = tVec2{w * k, 0};
            rotate(v, -60 * k);
            translate(v, offset);
            shape.Set(v.data(), v.size());
            b2FixtureDef fixture_def;
            fixture_def.shape = &shape;
            body->CreateFixture(&fixture_def);
        }


        return { entity };
    }

    std::tuple<entt::entity, Goal&> Goal::Generate(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry, tVec2 pos)
    {
        auto entity = registry->create();
        registry->emplace<Transform>(entity, pos, 0.0f);
        auto physics = service_locator->Get<Physics>();

        auto goal = registry->emplace<Goal>(entity);
        
        float d = 0.5f;

        b2BodyDef body_def;
        body_def.type = b2BodyType::b2_kinematicBody;

        auto body = physics->GenerateBody(entity, body_def)._body;
        b2PolygonShape shape;
        shape.SetAsBox(d / 2, d / 2);
        b2FixtureDef fixture_def;
        fixture_def.shape = &shape;
        fixture_def.isSensor = true;

        body->CreateFixture(&fixture_def);

        goal._frame = std::get<0>(GoalFrame::Generate(service_locator, registry, pos));

        return { entity, goal };
    }

    void generate_stage(observer_ptr<tofu::ServiceLocator> service_locator, observer_ptr<entt::registry> registry)
    {
        auto physics = service_locator->Get<Physics>();

        {
            // floor
            auto entity = registry->create();
            registry->emplace<Transform>(entity, tVec2{ 4.f, 6.f }, 0.0f);

            auto body = physics->GenerateBody(entity)._body;
            b2PolygonShape shape;
            shape.SetAsBox(4.0f, 0.1f);
            b2FixtureDef fixture_def;
            fixture_def.shape = &shape;
            fixture_def.density = 1.0f;
            fixture_def.friction = 0.9f;
            fixture_def.restitution = 0.1f;

            body->CreateFixture(&fixture_def);
            registry->emplace<Wall>(entity);
        }
        {
            // ceil
            auto entity = registry->create();
            registry->emplace<Transform>(entity, tVec2{ 4.f, 0.f }, 0.0f);

            auto body = physics->GenerateBody(entity)._body;
            b2PolygonShape shape;
            shape.SetAsBox(4.0f, 0.1f);
            b2FixtureDef fixture_def;
            fixture_def.shape = &shape;
            fixture_def.density = 1.0f;
            fixture_def.friction = 0.9f;
            fixture_def.restitution = 0.1f;

            body->CreateFixture(&fixture_def);
            registry->emplace<Wall>(entity);
        }
        {
            // Wall
            auto entity = registry->create();
            registry->emplace<Transform>(entity, tVec2{ 0.f, 3.f }, 0.0f);

            auto body = physics->GenerateBody(entity)._body;
            b2PolygonShape shape;
            shape.SetAsBox(0.1f, 3.0f);
            b2FixtureDef fixture_def;
            fixture_def.shape = &shape;
            fixture_def.density = 1.0f;
            fixture_def.friction = 0.9f;
            fixture_def.restitution = 0.1f;

            body->CreateFixture(&fixture_def);
            registry->emplace<Wall>(entity);
        }
        {
            // Wall
            auto entity = registry->create();
            registry->emplace<Transform>(entity, tVec2{ 8.f, 3.f }, 0.0f);

            auto body = physics->GenerateBody(entity)._body;
            b2PolygonShape shape;
            shape.SetAsBox(0.1f, 3.0f);
            b2FixtureDef fixture_def;
            fixture_def.shape = &shape;
            fixture_def.density = 1.0f;
            fixture_def.friction = 0.9f;
            fixture_def.restitution = 0.1f;

            body->CreateFixture(&fixture_def);
            registry->emplace<Wall>(entity);
        }
        {
            // Left-Slope
            auto entity = registry->create();
            float d = 0.4f;
            registry->emplace<Transform>(entity, tVec2{ d, 6 - d }, to_radians(-45.f));

            auto body = physics->GenerateBody(entity)._body;
            b2PolygonShape shape;
            shape.SetAsBox(0.1f, d + 0.1f);
            b2FixtureDef fixture_def;
            fixture_def.shape = &shape;
            fixture_def.density = 1.0f;
            fixture_def.friction = 0.9f;
            fixture_def.restitution = 0.1f;

            body->CreateFixture(&fixture_def);
            registry->emplace<Wall>(entity);
        }
        {
            // Right-Slope
            auto entity = registry->create();
            float d = 0.4f;
            registry->emplace<Transform>(entity, tVec2{ 8 - d, 6 - d }, to_radians(45.f));

            auto body = physics->GenerateBody(entity)._body;
            b2PolygonShape shape;
            shape.SetAsBox(0.1f, d + 0.1f);
            b2FixtureDef fixture_def;
            fixture_def.shape = &shape;
            fixture_def.density = 1.0f;
            fixture_def.friction = 0.9f;
            fixture_def.restitution = 0.1f;

            body->CreateFixture(&fixture_def);
            registry->emplace<Wall>(entity);
        }

        {
            // goal
            Goal::Generate(service_locator, registry, { 0.5f, 3.5f });
            Goal::Generate(service_locator, registry, { 7.5f, 3.5f });
        }
    }

}