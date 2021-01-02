#include <gtest/gtest.h>

#include "tofu/utils/tvec2.h"

TEST(Util_TVec2, ゼロ初期化される)
{
    tofu::tVec2 v;
    EXPECT_FLOAT_EQ(0, v._x);
    EXPECT_FLOAT_EQ(0, v._y);
}

