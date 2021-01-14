#pragma once

#include <memory>
#include <tofu/utils/observer_ptr.h>

namespace tofu
{
    // 循環キューのように振る舞うメモリアロケータ
    class CircularQueueAllocator
    {
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
        CircularQueueAllocator(std::size_t capacity)
            : _capacity(capacity)
        {
            _buffer = std::make_unique<std::byte[]>(capacity);

            _front = _back = _buffer.get();
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

            return res;
        }
        void Deallocate(std::byte* ptr)
        {
            _allocateCount--;

            ForwardTag* f_tag = reinterpret_cast<ForwardTag*>(ptr - sizeof(ForwardTag));
            f_tag->used = false;

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
            } while (!f_tag->used || reinterpret_cast<std::byte*>(f_tag) == _back);
            _front = reinterpret_cast<std::byte*>(f_tag);
        }
        bool CanAllocate(std::size_t size)
        {
            return CanAllocateBack(size) || CanAllocateHead(size);
        }
    private:
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
            return size <= space;
        }
    private:
        std::unique_ptr<std::byte[]> _buffer;
        const std::size_t _capacity;

        int _allocateCount = 0;
        std::byte* _front;
        std::byte* _back;
    };
}
