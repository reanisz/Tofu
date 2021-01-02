#include <gtest/gtest.h>
#include <atomic>
#include <condition_variable>

#include "tofu/utils/scheduled_update_thread.h"

TEST(Util_Scheduled_Update_Thread, スレッド生成してjoinできる)
{
    std::atomic<int> counter = 0;

    tofu::ScheduledUpdateThread scheduler{std::chrono::milliseconds{10}, [&](auto&){ counter++; }};
    scheduler.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds{30});
    while(counter < 3)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
    scheduler.End(true);
}

TEST(Util_Scheduled_Update_Thread, Startするまで実行されない)
{
    std::atomic<int> counter = 0;

    tofu::ScheduledUpdateThread scheduler{std::chrono::milliseconds{10}, [&](auto&){ counter++; }};
    std::this_thread::sleep_for(std::chrono::milliseconds{30});

    EXPECT_EQ(0, counter);

    scheduler.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds{30});
    while(counter < 3)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
    scheduler.End(true);
}

TEST(Util_Scheduled_Update_Thread, 実行せずに破棄できる)
{
    tofu::ScheduledUpdateThread scheduler{std::chrono::milliseconds{10}, [&](auto&){}};
}

