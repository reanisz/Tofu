#include <gtest/gtest.h>

#include "tofu/containers/stack_vector.h"

TEST(Container_StackVector, デフォルト初期化)
{
    tofu::stack_vector<int, 8> vec;
    EXPECT_EQ(0, vec.size());
}

TEST(Container_StackVector, 初期化リストによる初期化)
{
    tofu::stack_vector<int, 8> vec = { 1, 2, 3 };
    EXPECT_EQ(3, vec.size());
    EXPECT_EQ(1, vec[0]);
    EXPECT_EQ(2, vec[1]);
    EXPECT_EQ(3, vec[2]);
}

TEST(Container_StackVector, コピー初期化)
{
    tofu::stack_vector<int, 8> vec1 = { 1, 2, 3 };
    tofu::stack_vector<int, 8> vec2 = vec1;
    EXPECT_EQ(3, vec2.size());
    EXPECT_EQ(1, vec2[0]);
    EXPECT_EQ(2, vec2[1]);
    EXPECT_EQ(3, vec2[2]);
}

TEST(Container_StackVector, 初期化リストによる代入)
{
    tofu::stack_vector<int, 8> vec;
    EXPECT_EQ(0, vec.size());
    vec = { 1, 2, 3 };
    EXPECT_EQ(3, vec.size());
    EXPECT_EQ(1, vec[0]);
    EXPECT_EQ(2, vec[1]);
    EXPECT_EQ(3, vec[2]);
}

TEST(Container_StackVector, コピー代入)
{
    tofu::stack_vector<int, 8> vec1 = { 1, 2, 3 };
    tofu::stack_vector<int, 8> vec2;

    EXPECT_EQ(0, vec2.size());
    vec2 = vec1;

    EXPECT_EQ(3, vec2.size());
    EXPECT_EQ(1, vec2[0]);
    EXPECT_EQ(2, vec2[1]);
    EXPECT_EQ(3, vec2[2]);
}

TEST(Container_StackVector, range_based_for)
{
    tofu::stack_vector<int, 8> vec = {1, 2, 3};
    
    for(int& val : vec)
        val++;

    EXPECT_EQ(3, vec.size());
    EXPECT_EQ(2, vec[0]);
    EXPECT_EQ(3, vec[1]);
    EXPECT_EQ(4, vec[2]);
}

TEST(Container_StackVector, iterator)
{
    tofu::stack_vector<int, 8> vec = {1, 2, 3};
    
    int num = 1;
    for(auto it = vec.begin(); it != vec.end(); it++)
    {
        EXPECT_EQ(num, *it);
        num++;
    }
}

TEST(Container_StackVector, reverse_iterator)
{
    tofu::stack_vector<int, 8> vec = {1, 2, 3};
    
    int num = 3;
    for(auto it = vec.rbegin(); it != vec.rend(); it++)
    {
        EXPECT_EQ(num, *it);
        num--;
    }
}

TEST(Container_StackVector, 添字演算子)
{
    tofu::stack_vector<int, 8> vec = {1, 2, 3};
    
    EXPECT_EQ(1, vec[0]);
    EXPECT_EQ(2, vec[1]);
    EXPECT_EQ(3, vec[2]);
}

TEST(Container_StackVector, at)
{
    tofu::stack_vector<int, 8> vec = {1, 2, 3};
    
    EXPECT_EQ(1, vec.at(0));
    EXPECT_EQ(2, vec.at(1));
    EXPECT_EQ(3, vec.at(2));
}

TEST(Container_StackVector, push_back)
{
    tofu::stack_vector<int, 8> vec = {1, 2, 3};

    EXPECT_EQ(3, vec.size());
    EXPECT_EQ(1, vec[0]);
    EXPECT_EQ(2, vec[1]);
    EXPECT_EQ(3, vec[2]);
}

TEST(Container_StackVector, emplace_back)
{
    struct Sample {
        Sample() : _value(0) {}
        Sample(int x, int y) : _value(x * y) {}
        int _value;
    };
    tofu::stack_vector<Sample, 8> vec;
    vec.emplace_back(2, 3);
    vec.emplace_back(3, 4);
    vec.emplace_back(4, 5);

    EXPECT_EQ(3, vec.size());
    EXPECT_EQ(6, vec[0]._value);
    EXPECT_EQ(12, vec[1]._value);
    EXPECT_EQ(20, vec[2]._value);
}

TEST(Container_StackVector, pop_back)
{
    tofu::stack_vector<int, 8> vec = {1, 2, 3};
    vec.pop_back();

    EXPECT_EQ(2, vec.size());
    EXPECT_EQ(1, vec[0]);
    EXPECT_EQ(2, vec[1]);
}

TEST(Container_StackVector, 先頭へのinsert)
{
    tofu::stack_vector<int, 8> vec = {1, 2, 3};
    vec.insert(vec.begin(), 9);

    EXPECT_EQ(4, vec.size());
    EXPECT_EQ(9, vec[0]);
    EXPECT_EQ(1, vec[1]);
    EXPECT_EQ(2, vec[2]);
    EXPECT_EQ(3, vec[3]);
}

TEST(Container_StackVector, 途中へのinsert)
{
    tofu::stack_vector<int, 8> vec = {1, 2, 3};

    vec.insert(vec.begin() + 1, 9);

    EXPECT_EQ(4, vec.size());
    EXPECT_EQ(1, vec[0]);
    EXPECT_EQ(9, vec[1]);
    EXPECT_EQ(2, vec[2]);
    EXPECT_EQ(3, vec[3]);
}

TEST(Container_StackVector, 末尾へのinsert)
{
    tofu::stack_vector<int, 8> vec = {1, 2, 3};
    vec.insert(vec.end(), 9);

    EXPECT_EQ(4, vec.size());
    EXPECT_EQ(1, vec[0]);
    EXPECT_EQ(2, vec[1]);
    EXPECT_EQ(3, vec[2]);
    EXPECT_EQ(9, vec[3]);
}

TEST(Container_StackVector, 途中のerase)
{
    tofu::stack_vector<int, 8> vec = {1, 2, 3};

    vec.erase(vec.begin() + 1);

    EXPECT_EQ(2, vec.size());
    EXPECT_EQ(1, vec[0]);
    EXPECT_EQ(3, vec[1]);
}

TEST(Container_StackVector, 末尾のerase)
{
    tofu::stack_vector<int, 8> vec = {1, 2, 3};

    vec.erase(vec.end() - 1);

    EXPECT_EQ(2, vec.size());
    EXPECT_EQ(1, vec[0]);
    EXPECT_EQ(2, vec[1]);
}

TEST(Container_StackVector, 途中の範囲のerase)
{
    tofu::stack_vector<int, 8> vec = {1, 2, 3, 4, 5};

    vec.erase(vec.begin() + 1, vec.begin() + 4);

    EXPECT_EQ(2, vec.size());
    EXPECT_EQ(1, vec[0]);
    EXPECT_EQ(5, vec[1]);
}

TEST(Container_StackVector, 末尾までの範囲のerase)
{
    tofu::stack_vector<int, 8> vec = {1, 2, 3, 4, 5};

    vec.erase(vec.begin() + 2, vec.end());

    EXPECT_EQ(2, vec.size());
    EXPECT_EQ(1, vec[0]);
    EXPECT_EQ(2, vec[1]);
}

namespace {
    namespace stack_vector_test {
		static int ctor = 0;
		static int dtor = 0;

        struct Sample {
            Sample()
                : _deleted(false)
            {
                ctor++;
            }
            Sample(const Sample&)
                : _deleted(false)
            {
                ctor++;
            }
            ~Sample() {
                EXPECT_FALSE(_deleted);
                _deleted = true;
                dtor++;
            }

            bool _deleted;
        };

        void swap(Sample& lhs, Sample& rhs)
        {
            std::swap(lhs._deleted, rhs._deleted);
        }
    }
}
TEST(Container_StackVector, ctorとdtorの呼ばれる回数)
{
    using namespace stack_vector_test;
    ctor = dtor = 0;

    {
        tofu::stack_vector<Sample, 8> vec;
        vec.push_back(Sample{});
        vec.emplace_back();
        vec.emplace_back();
        vec.emplace_back();
        vec.emplace_back();

        vec.erase(vec.begin() + 1, vec.begin() + 3);

        tofu::stack_vector<Sample, 8> vec2;
        vec2 = vec;
        vec2.emplace_back();
    }

    EXPECT_EQ(ctor, dtor);
}
