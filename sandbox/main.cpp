#include <iostream>
#include <vector>
#include <string>

#include <fmt/core.h>

#include "quic_server.h"

struct CommandlineArguments
{
    CommandlineArguments(int argc, const char** argv)
        : _pos(0)
    {
        for(int i=0;i<argc;i++)
        {
            _args.emplace_back(argv[i]);
        }
    }

    const std::string& shift()
    {
        return _args[_pos++];
    }

    const std::string& operator[](std::ptrdiff_t idx) const
    {
        return _args[_pos + idx];
    }

    std::size_t remain() const
    {
        return _args.size() - _pos;
    }

private:
    std::vector<std::string> _args;
    int _pos;
};

void sandbox_server(CommandlineArguments args)
{
	using namespace tofu::net;

    Port port = std::stoi(args.shift());

    QuicServerConfig config = {
        ._config = {
            ._qlogDirectory = std::filesystem::path{"./qlog/"}
		},
        ._port = port,
        ._certFile = {"./cert/_wildcard.reanisz.info+3.pem"},
        ._secretFile = {"./cert/_wildcard.reanisz.info+3-key.pem"},
        ._alpn = "tofu_sandbox"
    };

    QuicServer quic{ config };
    quic.Start();
}

int main(int argc, const char** argv)
{
    auto args = CommandlineArguments(argc, argv);
    
    // argv[0] を読み飛ばす
    args.shift();

    auto& mode = args.shift();
    if(mode == "client")
    {
        fmt::print("Client mode\n");
    }
    else if(mode == "server")
    {
        fmt::print("Server mode\n");
        sandbox_server(args);
    }
    else 
    {
        fmt::print("Invalid Argument\n");
    }

    return 0;
}

