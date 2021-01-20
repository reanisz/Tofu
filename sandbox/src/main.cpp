#include <iostream>
#include <vector>
#include <string>

#include <fmt/core.h>

#include "tofu/net/quic_server.h"
#include "tofu/net/quic_client.h"
#include <tofu/utils/scheduled_update_thread.h>

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

    auto app = tofu::ScheduledUpdateThread{
        std::chrono::milliseconds{100},
        [&](const auto&)
        {
            quic.ForeachConnections([](tofu::net::QuicConnection& connection) {
                std::byte buf[65536];
                while (auto size = connection.ReadUnreliable(buf, sizeof(buf)))
                {
                    fmt::print("Receved Unreliable: {}\n", std::string_view{reinterpret_cast<char*>(buf), size});
					fmt::print("RTT: {}\n", picoquic_get_rtt(connection.GetRaw()));

                    std::string str{ reinterpret_cast<char*>(buf), size };
                    str += str;
                    connection.SendUnreliable(reinterpret_cast<const std::byte*>(str.c_str()), str.size());
                }

				if (auto stream = connection.GetStream(2))
				{
                    if (auto size = std::min<std::size_t>(stream->ReceivedSize(), sizeof(buf)))
                    {
                        stream->Read(buf, size);
						fmt::print("Receved Stream [{}]: {}\n", stream->GetId(), std::string_view{ reinterpret_cast<char*>(buf), size });
                    }
                }
            });
		}
    };

    app.Start();
	fmt::print("[sandbox_server] Press key to exit\n");
	getchar();

    app.End();
    quic.Exit();
    
    if (quic.HasError())
    {
        quic.GetError()->Dump();
    }
}

void sandbox_client(CommandlineArguments args)
{
	using namespace tofu::net;

    std::string address = args.shift();
    Port port = std::stoi(args.shift());

    QuicClientConfig config = {
        ._config = {
            ._qlogDirectory = std::filesystem::path{"./qlog/"}
		},
        ._serverName = address,
        ._port = port,
        ._alpn = "tofu_sandbox"
    };

    QuicClient quic{ config };
    quic.Start();

    auto connection = quic.GetConnection();

    std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });

    auto stream = connection->OpenStream(2);

    auto app = tofu::ScheduledUpdateThread{
        std::chrono::milliseconds{100},
        [&](const auto&)
        {
			std::byte buf[65536];
			while (auto size = connection->ReadUnreliable(buf, sizeof(buf)))
			{
				fmt::print("Receved Unreliable: {}\n", std::string_view{reinterpret_cast<char*>(buf), size});
				fmt::print("RTT: {}\n", picoquic_get_rtt(connection->GetRaw()));
			}
		}
    };
    app.Start();

    while (true) {
        fmt::print("> ");
        std::string line;
        std::cin >> line;
        fmt::print("\n");

        if (line == "exit")
            break;
        connection->SendUnreliable(reinterpret_cast<const std::byte*>(line.c_str()), line.size());
        stream->Send(reinterpret_cast<const std::byte*>(line.c_str()), line.size());

    }

    app.End();
    quic.Exit();
    
    if (quic.HasError())
    {
        quic.GetError()->Dump();
    }
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
        sandbox_client(args);
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

