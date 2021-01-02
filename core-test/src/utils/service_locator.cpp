#include <gtest/gtest.h>

#include "tofu/utils/service_locator.h"

TEST(Util_ServiceLocator, RegisterしてGetできる)
{
    tofu::ServiceLocator sl;

    sl.Register(std::make_unique<int>(10));
    sl.Register(std::make_unique<std::string>("hoge"));
    
    EXPECT_EQ(10, *sl.Get<int>());
    EXPECT_EQ(std::string{"hoge"}, *sl.Get<std::string>());
}

