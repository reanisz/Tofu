#pragma once

#include <optional>
#include <array>

namespace tofu {
    namespace ring_buffer
    {
        template<class TRingBuffer, class T, class difference_type>
        struct HeadOrigin
        {
            constexpr T& operator[](difference_type index) noexcept
            {
                return static_cast<TRingBuffer*>(this)->at_from_head(index);
            }
            constexpr const T& operator[](difference_type index) const noexcept
            {
                return static_cast<const TRingBuffer*>(this)->at_from_head(index);
            }
        };
        template<class TRingBuffer, class T, class difference_type>
        struct TailOrigin
        {
            constexpr T& operator[](difference_type index) noexcept
            {
                return static_cast<TRingBuffer*>(this)->at_from_tail(index);
            }
            constexpr const T& operator[](difference_type index) const noexcept
            {
                return static_cast<const TRingBuffer*>(this)->at_from_tail(index);
            }
        };
    }

    template<class T, std::size_t Capacity, template<class, class, class> class TOrigin = ring_buffer::HeadOrigin>
    class RingBuffer : public TOrigin<RingBuffer<T, Capacity, TOrigin>, T, std::size_t>
    {
    public:
        using reference = T&;
        using const_reference = const T&;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using value_type = T;

        constexpr RingBuffer() noexcept
            : _size(0)
            , _head(0)
        {
        }

        constexpr size_type size() const noexcept
        {
            return _size;
        }

        constexpr size_type capacity() const noexcept
        {
            return Capacity;
        }

        // headが0でtail方向に正に増えていくアクセサ
        constexpr T& at_from_head(difference_type index) noexcept
        {
            assert(0 <= index);
            return *_buffer[get_index(index)];
        }
        constexpr const T& at_from_head(difference_type index) const noexcept
        {
            assert(0 <= index);
            return *_buffer[get_index(index)];
        }

        // tailが0でhead方向に負に増えていくアクセサ
        constexpr T& at_from_tail(difference_type index) noexcept
        {
            assert(index <= 0);
            return *_buffer[get_index(_size - 1 + index)];
        }
        constexpr const T& at_from_tail(difference_type index) const noexcept
        {
            assert(index <= 0);
            return *_buffer[get_index(_size - 1 + index)];
        }

        template<class U = T>
        constexpr void push_back(U&& value)
            requires std::is_convertible_v<U, T>
        {
            assert(_size < Capacity);

            _buffer[get_index(_size)] = std::forward<U>(value);
            _size++;
        }

        template<class U = T>
        constexpr void push_front(U&& value)
            requires std::is_convertible_v<U, T>
        {
            assert(_size < Capacity);

            _buffer[get_index(-1)] = std::forward<U>(value);
            _size++;
            _head--;

            _head = (_head + Capacity) % Capacity;
        }

        constexpr T pop_front()
        {
            assert(0 < _size);

            _size--;
            _head++;
            _head = (_head + Capacity) % Capacity;
            return *std::exchange(_buffer[get_index(-1)], std::nullopt);
        }

        constexpr T pop_back()
        {
            assert(0 < _size);

            _size--;
            return *std::exchange(_buffer[get_index(_size)], std::nullopt);
        }

        constexpr void clear()
        {
            _size = 0;
            for (auto& val : _buffer)
            {
                val = std::nullopt;
            }
        }

    private:
        constexpr difference_type get_index(difference_type idx) const noexcept
        {
            idx %= Capacity;
            return (Capacity + _head + idx) % Capacity;
        }

    private:
        size_type _head;
        size_type _size;
        std::array<std::optional<T>, Capacity> _buffer;
    };

}

