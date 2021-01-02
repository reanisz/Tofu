#include <gtest/gtest.h>

#include "tofu/utils/observer_ptr.h"

TEST(Util_ObserverPtr, null初期化される)
{
    tofu::observer_ptr<int> ptr;
    EXPECT_EQ(nullptr, ptr);
}

TEST(Util_ObserverPtr, 生ポインタと相互変換できる)
{
    int value;

    int* raw = &value;
    tofu::observer_ptr<int> p = raw;

    EXPECT_EQ(&value, p.get());

    int *raw_2 = p.get();

    EXPECT_EQ(&value, raw_2);
}

TEST(Util_ObserverPtr, 間接参照できる)
{
    struct Sample{ int x; };

    Sample value { 99 };

    tofu::observer_ptr<Sample> ptr = &value;

    EXPECT_EQ(99, (*ptr).x);
    EXPECT_EQ(99, ptr->x);
}

TEST(Util_ObserverPtr, resetできる)
{
    int val_1, val_2;

    tofu::observer_ptr<int> ptr = &val_1;

    EXPECT_EQ(&val_1, ptr.get());

    ptr.reset();

    EXPECT_EQ(nullptr, ptr.get());

    ptr.reset(&val_2);

    EXPECT_EQ(&val_2, ptr.get());
}

