#pragma once

#include <Siv3D.hpp>
#include <entt/entt.hpp>
#include <box2d/box2d.h>

#include <tofu/utils.h>
#include <tofu/ecs/core.h>
#include <tofu/ecs/physics.h>
#include <tofu/renderer/siv3d.h>

namespace tofu 
{
    struct Box2DPrimitiveRenderer
    {
        Color _fillColor;
        Color _frameColor;
    };

    class Box2DPrimitiveRenderSystem
    {
    public:
        Box2DPrimitiveRenderSystem(observer_ptr<ServiceLocator> service_locator, observer_ptr<entt::registry> registry, float scale);

        float GetScale() const;
        void Render();

    private:
        void RenderShape(const Transform& transform, const Box2DPrimitiveRenderer& config, b2CircleShape* shape);
        void RenderShape(const Transform& transform, const Box2DPrimitiveRenderer& config, b2PolygonShape* shape);
        void RenderShape(const Transform& transform, const Box2DPrimitiveRenderer& config, b2EdgeShape* shape);
        void RenderShape(const Transform& transform, const Box2DPrimitiveRenderer& config, b2ChainShape* shape);

        template<class TVec>
        TVec rotate(const TVec& pos, float angle)
        {
            return Mat3x2::Rotate(angle).transform(pos);
        }

    private:
        observer_ptr<entt::registry> _registry;
        observer_ptr<S3DRenderSystem> _renderSystem;
        float _factor;
    };

    namespace jobs
    {
        class RenderBox2DPrimitives
        {
        public:
            RenderBox2DPrimitives(observer_ptr<Box2DPrimitiveRenderSystem> system)
                : _system(system)
            {
            }

            void operator()() const
            {
                _system->Render();
            }
        private:
            observer_ptr<Box2DPrimitiveRenderSystem> _system;
        };
    }
}

