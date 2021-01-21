#include <gtest/gtest.h>

#include "tofu/utils/circular_queue_allocator.h"

namespace
{
	// allocateするけど
	template<class T>
	T* allocate(tofu::CircularBufferAllocator& allocator)
	{
		T* ptr = reinterpret_cast<T*>(allocator.Allocate(sizeof(T)));
		return ptr;
	}

	template<class T>
	void deallocate(tofu::CircularBufferAllocator& allocator, T* ptr)
	{
		allocator.Deallocate(reinterpret_cast<std::byte*>(ptr));
	}
}

// TODO: 細分化する
TEST(Util_CircularBufferAllocator, 通常動作)
{
	tofu::CircularBufferAllocator allocator{ (sizeof(int) + tofu::CircularBufferAllocator::tag_size) * 3 };
	auto a = allocate<int>(allocator);
	auto b = allocate<int>(allocator);
	auto c = allocate<int>(allocator);
	auto d = allocate<int>(allocator); // 領域が足りなくなる
	EXPECT_TRUE(a);
	EXPECT_TRUE(b);
	EXPECT_TRUE(c);
	EXPECT_FALSE(d);

	deallocate(allocator, b);
	auto e = allocate<int>(allocator); // frontが解放されてないのでまだだめ

	EXPECT_FALSE(e);
	
	deallocate(allocator, a);
	auto f = allocate<int>(allocator);
	EXPECT_TRUE(f);
}
