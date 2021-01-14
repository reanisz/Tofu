#include <gtest/gtest.h>

#include "tofu/utils/circular_queue_allocator.h"

namespace
{
	// allocateÇ∑ÇÈÇØÇ«
	template<class T>
	T* allocate(tofu::CircularQueueAllocator& allocator)
	{
		T* ptr = reinterpret_cast<T*>(allocator.Allocate(sizeof(T)));
		return ptr;
	}

	template<class T>
	void deallocate(tofu::CircularQueueAllocator& allocator, T* ptr)
	{
		allocator.Deallocate(reinterpret_cast<std::byte*>(ptr));
	}
}

// TODO: ç◊ï™âªÇ∑ÇÈ
TEST(Util_CircularQueueAllocator, í èÌìÆçÏ)
{
	tofu::CircularQueueAllocator allocator{ (sizeof(int) + tofu::CircularQueueAllocator::tag_size) * 3 };
	auto a = allocate<int>(allocator);
	auto b = allocate<int>(allocator);
	auto c = allocate<int>(allocator);
	auto d = allocate<int>(allocator); // óÃàÊÇ™ë´ÇËÇ»Ç≠Ç»ÇÈ
	EXPECT_TRUE(a);
	EXPECT_TRUE(b);
	EXPECT_TRUE(c);
	EXPECT_FALSE(d);

	deallocate(allocator, b);
	auto e = allocate<int>(allocator); // frontÇ™âï˙Ç≥ÇÍÇƒÇ»Ç¢ÇÃÇ≈Ç‹ÇæÇæÇﬂ

	EXPECT_FALSE(e);
	
	deallocate(allocator, a);
	auto f = allocate<int>(allocator);
	EXPECT_TRUE(f);
}
