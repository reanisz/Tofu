#include <iostream>
#include <vector>
#include <string>

#include <fmt/core.h>

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
    }
    else 
    {
        fmt::print("Invalid Argument\n");
    }

    return 0;
}

