#include <gtest/gtest.h>

#include "tofu/utils/strong_numeric.h"

namespace
{
    using SampleInt = tofu::StrongNumeric<class tag_test_sample_int, int>;
    using SampleInt2 = tofu::StrongNumeric<class tag_test_sample_int_2, int>;

    using SampleDouble = tofu::StrongNumeric<class tag_test_sample_dobule, double>;
}

TEST(Util_StrongNumeric, 基本の演算)
{
    SampleInt a = 20;
    SampleInt b = 6;

    EXPECT_EQ(26, a + b);
    EXPECT_EQ(14, a - b);
    EXPECT_EQ(120, a * b);
    EXPECT_EQ(3, a / b);
    EXPECT_EQ(2, a % b);

    EXPECT_EQ(+6, +b);
    EXPECT_EQ(-6, -b);

    a++;
    EXPECT_EQ(21, a);
    a--;
    EXPECT_EQ(20, a);
}

TEST(Util_StrongNumeric, 浮動小数点型の演算)
{
    SampleDouble a = 1.5;
    SampleDouble b = 2.2;

    EXPECT_DOUBLE_EQ(3.7,       static_cast<double>(a + b));
    EXPECT_DOUBLE_EQ(-0.7,      static_cast<double>(a - b));
    EXPECT_DOUBLE_EQ(3.3,       static_cast<double>(a * b));
    EXPECT_DOUBLE_EQ((1.5/2.2), static_cast<double>(a / b));
}

#if 0
// NOTE: これはコンパイルエラーになることが望ましい
TEST(Util_StrongNumreic, コンパイルエラーになる式)
{
    SampleInt a;
    SampleInt2 b;

    a = b;
}
#endif

