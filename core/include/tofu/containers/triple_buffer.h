#pragma once

namespace tofu {
    // TODO: テストを書く
    // TODO: 説明を書く
    template<class T>
    class TripleBuffer
    {
        enum class State 
        {
            Invalid, // 有効なデータが入ってない
            Writing, // 書き込み中
            Valid,   // 有効なデータが入っていて読み取れる
            Reading, // 読み取り中
            Used,    // 一度読み取った
        };
        struct Buffer 
        {
            // コピー・ムーブ禁止
            Buffer() = default;
            Buffer(const Buffer&) = delete;
            Buffer(Buffer&&) = delete;
            Buffer& operator=(const Buffer&) = delete;
            Buffer& operator=(Buffer&&) = delete;

            T _data;
            State _state;
        };
    public:
        class Ownership
        {
        public:
            using pointer = T*;
            using element_type = T;

            constexpr Ownership() noexcept
                : _parent(nullptr)
                , _buffer(nullptr)
            {
            }
            constexpr Ownership(observer_ptr<TripleBuffer> parent, observer_ptr<Buffer> buffer) noexcept
                : _parent(parent)
                , _buffer(buffer)
            {
            }

            ~Ownership()
            {
                Disown();
            }

            // コピー禁止
            Ownership(const Ownership&) = delete;
            Ownership& operator=(const Ownership&) = delete;

            constexpr Ownership(Ownership&& other) noexcept
            {
                *this = std::move(other);
            }
            constexpr Ownership& operator=(Ownership&& other) noexcept
            {
                Disown();
                std::swap(_parent, other._parent);
                std::swap(_buffer, other._buffer);

                return *this;
            }

            constexpr void Disown() noexcept
            {
                if (_parent && _buffer)
                {
                    _parent->Disown(_buffer);
                }
                _parent = nullptr;
                _buffer = nullptr;
            }

            constexpr element_type& operator*() noexcept
            {
                return _buffer->_data;
            }
            constexpr const element_type& operator*() const noexcept
            {
                return _buffer->_data;
            }

            constexpr pointer operator->() noexcept
            {
                return &_buffer->_data;
            }
            constexpr const pointer operator->() const noexcept
            {
                return &_buffer->_data;
            }


        private:
            observer_ptr<TripleBuffer> _parent;
            observer_ptr<Buffer> _buffer;
        };

        TripleBuffer()
        {
            for (auto& buffer : _buffers)
            {
                buffer._state = State::Invalid;
            }
        }

        bool CanRead() const noexcept
        {
            return findForState(State::Valid);
        }
        bool CanReadUsed() const noexcept
        {
            return findForState(State::Used);
        }

        // 書き込み者として所有権を得る。実行中ロックされる
        Ownership ownAsWriter()
        {
            std::lock_guard lock(_mutex);
            auto buffer = findForState(State::Invalid);
            if (!buffer)
                buffer = findForState(State::Used);
            assert(buffer);
            buffer->_state = State::Writing;
            return Ownership{this, buffer};
        }
        // 読み込み者として所有権を得る。実行中ロックされる
        Ownership ownAsReader()
        {
            std::lock_guard lock(_mutex);
            auto buffer = findForState(State::Valid);
            assert(buffer);
            buffer->_state = State::Reading;
            return Ownership{this, buffer};
        }
        // 読み込み者として所有権を得る。実行中ロックされる。最新データがなければ前と同じデータを読む
        Ownership ownAsReaderAllowUsed()
        {
            std::lock_guard lock(_mutex);
            auto buffer = findForState(State::Valid);
            if (!buffer)
                buffer = findForState(State::Used);
            assert(buffer);
            buffer->_state = State::Reading;
            return Ownership{this, buffer};
        }

    private:
        Buffer* findForState(State state)
        {
            for (auto& buffer : _buffers)
            {
                if (buffer._state == state)
                    return &buffer;
            }
            return nullptr;
        }
        const Buffer* findForState(State state) const
        {
            for (auto& buffer : _buffers)
            {
                if (buffer._state == state)
                    return &buffer;
            }
            return nullptr;
        }

        // 所有権を返す。実行中ロックされる
        void Disown(observer_ptr<Buffer> buffer) 
        {
            std::lock_guard lock(_mutex);

            switch (buffer->_state)
            {
            case State::Writing:
            {
                // 古いデータは無効にしておく
                {
                    auto old_data = findForState(State::Valid);
                    if (old_data)
                        old_data->_state = State::Invalid;
                }
                {
                    auto old_data = findForState(State::Used);
                    if (old_data)
                        old_data->_state = State::Invalid;
                }

                buffer->_state = State::Valid;
            }
                break;
            default:
            case State::Reading:
                if (findForState(State::Valid))
                    buffer->_state = State::Invalid;
                else
                    buffer->_state = State::Used;
                break;
            }
        }

        std::array<Buffer, 3> _buffers;
        std::mutex _mutex;
    };
}

