#pragma once

#include <tofu/utils.h>
#include <tofu/containers.h>

namespace tofu
{
    template<class TRenderVariant>
    struct RenderCommand
    {
        TRenderVariant _command;
        float _priority; // 優先度(小さい方が先)
    };

    template<class TRenderVariant>
    class RenderQueue
    {
    public:
        using command_type = RenderCommand<TRenderVariant>;

        void Push(const command_type& command)
        {
            _commands.push_back(command);
        }

        void Clear()
        {
            _commands.clear();
        }

        template<class TRenderer>
        void Render(TRenderer& renderer)
        {
            // std::ranges::stable_sortがMSVCでまだ実装されてなかった。。。
            std::stable_sort(_commands.begin(), _commands.end(), [](const command_type& lhs, const command_type& rhs) { return lhs._priority < rhs._priority; });
            for (auto& command : _commands)
            {
                std::visit([&](auto& cmd) { renderer(cmd); }, command._command);
            }
        }

    private:
        std::vector<command_type> _commands;
    };

    template<class TRenderVariant, class TRenderer>
    class RenderSystem
    {
    public:
        using queue_type = RenderQueue<TRenderVariant>;
        using ownership = typename TripleBuffer<queue_type>::Ownership;
        using command_type = RenderCommand<TRenderVariant>;

        void StartWrite()
        {
            _writeQueue = _queues.ownAsWriter();
            _writeQueue->Clear();
        }
        void Enqueue(const TRenderVariant& command, float priority)
        {
            _writeQueue->Push(command_type{ command, priority });
        }
        void EndWrite()
        {
            _writeQueue.Disown();
        }

        bool HasData() const
        {
            return _queues.CanRead() || _queues.CanReadUsed();
        }

        void Render()
        {
            _readQueue = _queues.ownAsReaderAllowUsed();
            _readQueue->Render(_renderer);
            _readQueue.Disown();
        }

    private:
        TripleBuffer<queue_type> _queues;
        TRenderer _renderer;

        ownership _readQueue;
        ownership _writeQueue;
    };

    namespace jobs
    {
        template<class TRenderSystem>
        class StartRender
        {
        public:
            StartRender(observer_ptr<TRenderSystem> system)
                : _system(system)
            {
            }

            void operator()() const
            {
                _system->StartWrite();
            }

        private:
            observer_ptr<TRenderSystem> _system;
        };

        template<class TRenderSystem>
        class EndRender
        {
        public:
            EndRender(observer_ptr<TRenderSystem> system)
                : _system(system)
            {
            }

            void operator()() const
            {
                _system->EndWrite();
            }

        private:
            observer_ptr<TRenderSystem> _system;
        };
    }

}

