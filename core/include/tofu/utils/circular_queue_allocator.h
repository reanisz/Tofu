#pragma once

#include <memory>
#include <tofu/utils/observer_ptr.h>

namespace tofu
{
    // �z�L���[�̂悤�ɐU�镑���������A���P�[�^
    class CircularQueueAllocator
    {
        struct ForwardTag
        {
            bool used : 1; // �g�p���Ȃ�true
            bool is_next_head : 1; // ���f�[�^�͐擪
            unsigned short size : 14;
        };
        struct BackwardTag
        {
            unsigned short size : 14;
        };
    public:
        static constexpr std::size_t tag_size = sizeof(ForwardTag) + sizeof(BackwardTag);

        // capacity�̓f�[�^�����܂鐄��ő�ʂ�2�{���炢���ڈ�
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
                // �z��������t���O�𗧂ĂĂ���
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
                // ����͌��
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
        // back�̌��ɂ��̂܂܊m�ۉ\
        bool CanAllocateBack(std::size_t size)
        {
            std::intptr_t used = reinterpret_cast<std::intptr_t>(_back) - reinterpret_cast<std::intptr_t>(_buffer.get());
            return size <= _capacity - used;
        }
        // head�Ɋm�ۉ\
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
