#pragma once

#include <Siv3D.hpp>
#include <tofu/renderer.h>

namespace tofu
{
    namespace render_command
    {
        template<class TShape>
        struct S3DShapeFill
        {
            constexpr S3DShapeFill(const TShape& shape) noexcept
                : _shape(shape)
                , _color(Palette::Black)
            {
            }

            constexpr S3DShapeFill(const TShape& shape, Color color) noexcept
                : _shape(shape)
                , _color(color)
            {
            }

            TShape _shape;
            Color _color;
        };
        template<class TShape>
        struct S3DShapeFrame
        {
            constexpr S3DShapeFrame(const TShape& shape) noexcept
                : _shape(shape)
                , _color(Palette::Black)
                , _tickness(1)
            {
            }

            constexpr S3DShapeFrame(const TShape& shape, Color color) noexcept
                : _shape(shape)
                , _color(color)
                , _tickness(1)
            {
            }

            constexpr S3DShapeFrame(const TShape& shape, Color color, int tickness) noexcept
                : _shape(shape)
                , _color(color)
                , _tickness(tickness)
            {
            }

            TShape _shape;
            Color _color;
            int _tickness;
        };
    }
    namespace renderer
    {
        template<class TShape>
        class S3DShapeFill
        {
        public:
            void operator()(const render_command::S3DShapeFill<TShape>& command)
            {
                command._shape.draw(command._color);
            }
        };

        template<class TShape>
        class S3DShapeFrame
        {
        public:
            void operator()(const render_command::S3DShapeFrame<TShape>& command)
            {
                command._shape.drawFrame(command._tickness, command._color);
            }
        };
    }


    using S3DRenderCommandVariant = 
        std::variant<
              render_command::S3DShapeFill<Circle>
            , render_command::S3DShapeFrame<Circle>
            , render_command::S3DShapeFill<s3d::Polygon>
            , render_command::S3DShapeFrame<s3d::Polygon>
            , render_command::S3DShapeFill<Line>
        >;

    class S3DRenderer
        : public renderer::S3DShapeFill<Circle>
        , public renderer::S3DShapeFrame<Circle>
        , public renderer::S3DShapeFill<s3d::Polygon>
        , public renderer::S3DShapeFrame<s3d::Polygon>
        , public renderer::S3DShapeFill<Line>
    {
    };

    using S3DRenderSystem = RenderSystem<S3DRenderCommandVariant, S3DRenderer>;

    namespace jobs
    {
        using S3DStartRender = StartRender<S3DRenderSystem>;
        using S3DEndRender = EndRender<S3DRenderSystem>;
    }
}

