#include <filesystem>

#include <tofu/utils/strong_numeric.h>

#include <picoquic.h>

namespace tofu::net
{
    using Port = StrongNumeric<class tag_Port, int>;

    struct QuicConfig
    {
        std::filesystem::path _qlogDirectory;
    };

    struct QuicServerConfig
    {
        QuicConfig _config;

        Port _port;
        std::filesystem::path _certFile;
        std::filesystem::path _secretFile;
        std::string _alpn;
    };

    class QuicServer
    {
    public:
        QuicServer(const QuicServerConfig& config);

        void Start();

    private:
        QuicServerConfig _config;
    };
}
