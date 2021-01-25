#include <fmt/core.h>

#include <tofu/ball/network.h>
#include <tofu/ball/net_server.h>

namespace tofu::ball
{
    void run_server()
    {
        Server server;
        server.Run();
    }
}

int main(int argc, char** argv)
{
    tofu::ball::run_server();
}

