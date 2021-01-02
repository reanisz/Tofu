#include <gtest/gtest.h>

#include "tofu/utils.h"

TEST(RingBufferTest, pushとpopの基本的な動作)
{
    tofu::RingBuffer<int, 4> rb;
    rb.push_back(1);
    rb.push_back(2);
    rb.push_back(3);

    EXPECT_EQ(3, rb.size());
    EXPECT_EQ(1, rb[0]);
    EXPECT_EQ(2, rb[1]);
    EXPECT_EQ(3, rb[2]);


    rb.pop_front();
    rb.push_back(4);
    rb.push_back(5);

    EXPECT_EQ(4, rb.size());
    EXPECT_EQ(2, rb[0]);
    EXPECT_EQ(3, rb[1]);
    EXPECT_EQ(4, rb[2]);
    EXPECT_EQ(5, rb[3]);

    rb.pop_back();
    rb.pop_back();

    EXPECT_EQ(2, rb.size());
    EXPECT_EQ(2, rb[0]);
    EXPECT_EQ(3, rb[1]);
}


