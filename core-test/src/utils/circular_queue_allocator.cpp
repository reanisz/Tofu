#include <gtest/gtest.h>

#include "tofu/utils/circular_queue_allocator.h"

namespace
{
	// allocate���邯��
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

// TODO: �ו�������
TEST(Util_CircularBufferAllocator, �ʏ퓮��)
{
	tofu::CircularBufferAllocator allocator{ (sizeof(int) + tofu::CircularBufferAllocator::tag_size) * 3 };
	auto a = allocate<int>(allocator);
	auto b = allocate<int>(allocator);
	auto c = allocate<int>(allocator);
	auto d = allocate<int>(allocator); // �̈悪����Ȃ��Ȃ�
	EXPECT_TRUE(a);
	EXPECT_TRUE(b);
	EXPECT_TRUE(c);
	EXPECT_FALSE(d);

	deallocate(allocator, b);
	auto e = allocate<int>(allocator); // front���������ĂȂ��̂ł܂�����

	EXPECT_FALSE(e);
	
	deallocate(allocator, a);
	auto f = allocate<int>(allocator);
	EXPECT_TRUE(f);
}
