#pragma once

#include <cassert>
#include <memory>
#include <algorithm>
#include <tofu/utils/observer_ptr.h>

namespace tofu
{
    class CircularBufferAllocator
    {
    protected:
        struct ForwardTag
        {
            bool used : 1; // 使用中ならtrue
            bool is_next_head : 1; // 次データは先頭
            unsigned short size : 14;
        };
        struct BackwardTag
        {
            unsigned short size : 14;
        };
    public:
        static constexpr std::size_t tag_size = sizeof(ForwardTag) + sizeof(BackwardTag);

        // capacityはデータが溜まる推定最大量の2倍くらいが目安
        CircularBufferAllocator(std::size_t capacity)
            : _capacity(capacity)
        {
            _buffer = std::make_unique<std::byte[]>(capacity);
            _front = _back = _buffer.get();
        }
        // コピー禁止・ムーブ許可
        CircularBufferAllocator(const CircularBufferAllocator&) = delete;
        CircularBufferAllocator(CircularBufferAllocator&& other) = default;

        bool CanAllocate(std::size_t size)
        {
            return CanAllocateBack(size) || CanAllocateHead(size);
        }

        std::byte* Allocate(std::size_t size)
        {
            std::byte* base;

            if (CanAllocateBack(size))
            {
                base = _back;
            }
            else if (CanAllocateHead(size))
            {
                base = _front;
                // 循環させたよフラグを立てておく
                BackwardTag* b_tag = reinterpret_cast<BackwardTag*>(_back - sizeof(BackwardTag));
                ForwardTag* f_tag = reinterpret_cast<ForwardTag*>(_back - sizeof(BackwardTag) - b_tag->size - sizeof(ForwardTag));
                f_tag->is_next_head = true;
            }
            else
            {
                // full
                return nullptr;
            }

            ForwardTag* f_tag = reinterpret_cast<ForwardTag*>(base);
            BackwardTag* b_tag = reinterpret_cast<BackwardTag*>(base + sizeof(ForwardTag) + size);

            std::byte* res = base + sizeof(ForwardTag);

            std::byte* next_back = base + sizeof(ForwardTag) + size + sizeof(BackwardTag);

            f_tag->used = true;
            f_tag->is_next_head = false;
            f_tag->size = size;

            b_tag->size = size;

            _allocateCount++;
            _back = next_back;

            _size += size;

            return res;
        }
        void Deallocate(const std::byte* const_ptr)
        {
            auto ptr = const_cast<std::byte*>(const_ptr);
            _allocateCount--;

            ForwardTag* f_tag = reinterpret_cast<ForwardTag*>(ptr - sizeof(ForwardTag));
            f_tag->used = false;

            _size -= f_tag->size;

            if (reinterpret_cast<std::byte*>(f_tag) != _front)
            {
                // 解放は後回し
                return;
            }

            do
            {
                if (f_tag->is_next_head)
                {
                    f_tag = reinterpret_cast<ForwardTag*>(_buffer.get());
                }
                else {
                    f_tag = reinterpret_cast<ForwardTag*>(reinterpret_cast<std::byte*>(f_tag) + f_tag->size + sizeof(ForwardTag) + sizeof(BackwardTag));
                }
            } while (reinterpret_cast<std::byte*>(f_tag) != _back && !f_tag->used);
            _front = reinterpret_cast<std::byte*>(f_tag);
        }
    protected:
        // backの後ろにそのまま確保可能
        bool CanAllocateBack(std::size_t size)
        {
            std::intptr_t used = reinterpret_cast<std::intptr_t>(_back) - reinterpret_cast<std::intptr_t>(_buffer.get());
            return size <= _capacity - used;
        }
        // headに確保可能
        bool CanAllocateHead(std::size_t size)
        {
            std::intptr_t space = reinterpret_cast<std::intptr_t>(_front) - reinterpret_cast<std::intptr_t>(_buffer.get());
            return size <= static_cast<std::size_t>(space);
        }

    protected:
        std::unique_ptr<std::byte[]> _buffer;
        const std::size_t _capacity;

        // allocatedなブロック数 (deallocateしたら減る)
        std::size_t _allocateCount = 0;

        // 総サイズ
        std::size_t _size = 0;

        // allocatedなブロックの先頭 (deallocateしたら進む)
        std::byte* _front;
        // allocatedなブロックの最後尾 (allocateしたら進む)
        std::byte* _back;
    };

    // 循環メモリバッファー内で任意サイズのデータを格納できるキュー
    class CircularQueueBuffer : private CircularBufferAllocator
    {
    public:
        // capacityはデータが溜まる推定最大量の2倍くらいが目安
        CircularQueueBuffer(std::size_t capacity)
            : CircularBufferAllocator(capacity)
        {
            _iterator = _buffer.get();
        }
        // コピー禁止・ムーブ許可
        CircularQueueBuffer(const CircularQueueBuffer&) = delete;
        CircularQueueBuffer(CircularQueueBuffer&& other) = default;

        bool CanWrite(std::size_t size)
        {
            return CanAllocate(size);
        }

        void Write(const std::byte* const data, std::size_t length)
        {
            assert(CanWrite(length));
            if (!CanWrite(length)) {
                return;
            }

            auto buf = Allocate(length);
            _count++;
            memcpy(buf, data, length);
        }

        // 格納されているデータ数
        std::size_t Count() const
        {
            return _count;
        }

        struct PeekedData
        {
            PeekedData(std::byte* data, std::size_t length)
                : _data(data)
                , _length(length)
            {
            }

            const std::byte* const _data;
            const std::size_t _length;
        };

        struct ReadedData : public PeekedData
        {
            ReadedData(CircularQueueBuffer* parent, std::byte* data, std::size_t length)
                : PeekedData(data, length)
                , _parent(parent)
            {
            }

            ~ReadedData()
            {
                _parent->Deallocate(_data);
            }

            ReadedData(const ReadedData&) = delete;
            ReadedData(ReadedData&&) = delete;

            void CopyTo(std::byte* dest) const noexcept
            {
                memcpy(dest, _data, _length);
            }

            void Discard()
            {
                if (!_isDeallocated)
                {
                    _parent->Deallocate(_data);
                    _isDeallocated = true;
                }
            }

            bool _isDeallocated = false;

        private:
            CircularQueueBuffer* _parent;
        };

        PeekedData Peek()
        {
            assert(_count);
            auto it = _iterator;
            auto tag = reinterpret_cast<ForwardTag*>(it);
            auto data = it + sizeof(ForwardTag);

            return PeekedData{ data, tag->size };
        }

        ReadedData Pop()
        {
            assert(_count);
            auto it = _iterator;
            auto tag = reinterpret_cast<ForwardTag*>(it);
            auto data = it + sizeof(ForwardTag);

            if (!tag->is_next_head)
            {
                _iterator += tag->size + sizeof(ForwardTag) + sizeof(BackwardTag);
            }
            else 
            {
                _iterator = _buffer.get();
            }

            _count--;

            return ReadedData{this, data, tag->size};
        }

    private:
        // queueとして見たときのcount (popしたら減る)
        std::size_t _count = 0;

        // queueとして見たときの先頭 (popしたら進む)
        std::byte* _iterator;
    };

    class CircularContinuousBuffer
    {
    public:
        // capacity: アロケータ容量
        CircularContinuousBuffer(std::size_t capacity)
            : _capacity(capacity)
        {
            _buffer = std::make_unique<std::byte[]>(capacity);

            _front = _back = _buffer.get();
        }

        // コピー禁止・ムーブ許可
        CircularContinuousBuffer(const CircularContinuousBuffer&) = delete;
        CircularContinuousBuffer(CircularContinuousBuffer&& other) = default;

        std::size_t Remain() const noexcept
        {
            return _capacity - _size;
        }

        std::size_t Capacity() const noexcept
        {
            return _capacity;
        }

        std::size_t Size() const noexcept
        {
            return _size;
        }

        bool CanWrite(std::size_t length) const noexcept
        {
            return length <= Remain();
        }

        void Write(const std::byte* const data, std::size_t length)
        {
            assert(CanWrite(length));
            if (!CanWrite(length)) {
                return;
            }

            auto blength = std::min<std::size_t>(length, RemainBackward());
            auto flength = length - blength;
            if (0 < blength) 
            {
                memcpy(_back, data, blength);
                _back += length;
            }

            if (0 < flength)
            {
                memcpy(_buffer.get(), data + blength, flength);
                _back = _buffer.get() + flength;
            }

            _size += length;
        }

        // 先頭からlength byte分見る。見たデータは破棄されない
        void Peek(std::byte* write_to, std::size_t length) const
        {
            assert(length <= _size);

            auto blength = std::min<std::size_t>(length, UsedBackward());
            auto flength = std::min<std::size_t>(length - blength, UsedForward());

            memcpy(write_to, _front, blength);
            if (flength)
            {
                memcpy(write_to + flength, _buffer.get(), flength);
            }
        }

        // 先頭からlength byte分見て、そのデータは破棄される
        void Read(std::byte* write_to, std::size_t length)
        {
            Peek(write_to, length);
            Seek(length);
        }

        // length byte分読み終わった
        void Seek(std::size_t length)
        {
            auto backward_length = std::min<std::size_t>(length, RemainBackward());
            auto forward_length = std::min<std::size_t>(length - backward_length, RemainForward());

            if (forward_length == 0)
            {
                _front += backward_length;
            }
            else
            {
                _front = _buffer.get() + backward_length;
            }

            _size -= length;
        }

    private:
        // 後ろ方向の空き容量
        std::size_t RemainBackward() const noexcept
        {
            return reinterpret_cast<std::intptr_t>(_buffer.get() + _capacity) - reinterpret_cast<std::intptr_t>(_back);
        }

        // 前方向の空き容量
        std::size_t RemainForward() const noexcept
        {
            return reinterpret_cast<std::intptr_t>(_front) - reinterpret_cast<std::intptr_t>(_buffer.get());
        }

        // 後ろ方向の使用容量
        std::size_t UsedBackward() const noexcept
        {
            if (_front < _back)
            {
                return reinterpret_cast<std::intptr_t>(_back) - reinterpret_cast<std::intptr_t>(_front);
            }
            else 
            {
                return reinterpret_cast<std::intptr_t>(_buffer.get() + _capacity) - reinterpret_cast<std::intptr_t>(_front);
            }
        }

        // 前方向の使用容量
        std::size_t UsedForward() const noexcept
        {
            if (_front < _back)
            {
                return 0;
            }
            else 
            {
                return reinterpret_cast<std::intptr_t>(_back) - reinterpret_cast<std::intptr_t>(_buffer.get());
            }
        }

    private:
        const std::size_t _capacity;

        std::byte* _front;
        std::byte* _back;

        // 使用サイズ
        std::size_t _size = 0;

        // データを格納するバッファ
        std::unique_ptr<std::byte[]> _buffer;
    };
}
