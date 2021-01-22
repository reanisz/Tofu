#include <gtest/gtest.h>

#include "tofu/utils/tvec2.h"

#define EXPECT_TVEC2_EQ(expect_x, expect_y, actual) \
    { \
        auto ret = actual; \
        EXPECT_FLOAT_EQ(expect_x, ret._x); \
        EXPECT_FLOAT_EQ(expect_y, ret._y); \
    }


TEST(Util_tVec2, ゼロ初期化される)
{
    tofu::tVec2 v;
    EXPECT_FLOAT_EQ(0, v._x);
    EXPECT_FLOAT_EQ(0, v._y);
}

TEST(Util_tVec2, 基本的な演算)
{
    tofu::tVec2 x = { 1, 2 };
    tofu::tVec2 y = { 3, 4 };

    EXPECT_TVEC2_EQ(4, 6, x + y);
    EXPECT_TVEC2_EQ(-2, -2, x - y);
    EXPECT_TVEC2_EQ(-1, -2, -x);
    EXPECT_TVEC2_EQ(2, 4, 2 * x);
    EXPECT_TVEC2_EQ(2, 4, x * 2);
    EXPECT_TVEC2_EQ(2, 4, 2 * x);
    EXPECT_TVEC2_EQ(1.5f, 2, y / 2);

    EXPECT_FLOAT_EQ(1 * 1 + 2 * 2, x.LengthSquared());
    EXPECT_FLOAT_EQ(std::sqrt(1 * 1 + 2 * 2), x.Length());
}

TEST(Util_tVec2, 長さの計算関連)
{
    tofu::tVec2 x = { 1, 2 };
    tofu::tVec2 y = { 3, 3 };

    EXPECT_FLOAT_EQ(1 * 1 + 2 * 2, x.LengthSquared());
    EXPECT_FLOAT_EQ(std::sqrt(1 * 1 + 2 * 2), x.Length());

    EXPECT_TVEC2_EQ(std::sqrt(2) / 2, std::sqrt(2) / 2, y.Normalized());
}

