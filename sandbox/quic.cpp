#include "quic.h"

#include <iostream>
#include <array>
#include <fstream>
#include <cassert>

extern "C"
{
    void MsQuicLibraryLoad(void);
}


namespace quic
{
    const QUIC_API_TABLE* MsQuic;
    const QUIC_REGISTRATION_CONFIG RegConfig = { "quicsample", QUIC_EXECUTION_PROFILE_LOW_LATENCY };

    const QUIC_BUFFER _alpn = { sizeof("sample") - 1, (uint8_t*)"sample" };

    HQUIC _registration;
    HQUIC _configuration;

    void init()
    {
        MsQuicLibraryLoad();

        QUIC_STATUS status = QUIC_STATUS_SUCCESS;

        if(QUIC_FAILED(status = MsQuicOpen(&MsQuic)))
        {
            std::cout << "Error at MsQuicOpen : " << status << std::endl;
            exit(1);
        }

        if(QUIC_FAILED(status = MsQuic->RegistrationOpen(&RegConfig, &_registration)))
        {
            std::cout << "Error at RegistrationOpen: " << status << std::endl;
            exit(1);
        }

    }

#ifdef WIN32 
	void parse_hash_dat(std::byte* write_to, const std::string& file_name)
    {
        std::string line;
        {
            std::ifstream ifs{ file_name };
            ifs >> line;
        }
        if (line.size() != 2 * 20 + 19)
        {
            std::cout << "Error: hash file is invalid." << std::endl;
            exit(1);
        }

        auto parse = [](char c) -> std::byte
        {
            if ('0' <= c && c <= '9')
				return static_cast<std::byte>(c - '0');
            else if ('A' <= c && c <= 'F')
				return static_cast<std::byte>(10 + c - 'A');
            assert(false);
            return std::byte{ 0 };
        };

        for (int i = 0; i < 20; i++) 
        {
            char c1 = line[i * 3 + 0];
            char c2 = line[i * 3 + 1];

            write_to[i] = (parse(c1) << 4) | parse(c2);
        }
    }
#endif

#ifdef WIN32 
    void init_configuration(HQUIC& registration, HQUIC& configuration, bool is_server, const char* hash_file)
#else
    void init_configuration(HQUIC& registration, HQUIC& configuration, bool is_server, const char* cert, const char* private_key)
#endif
    {
        QUIC_SETTINGS settings{0};

        settings.IdleTimeoutMs = 1000;
        settings.IsSet.IdleTimeoutMs = true;

        if(is_server){
            settings.ServerResumptionLevel = QUIC_SERVER_RESUME_AND_ZERORTT;
            settings.IsSet.ServerResumptionLevel = true;

            settings.PeerBidiStreamCount = 1;
            settings.IsSet.PeerBidiStreamCount = true;
        }

        struct QUIC_CREDENTIAL_CONFIG_HELPER {
            QUIC_CREDENTIAL_CONFIG CredConfig;
            union {
                QUIC_CERTIFICATE_HASH CertHash;
                QUIC_CERTIFICATE_HASH_STORE CertHashStore;
                QUIC_CERTIFICATE_FILE CertFile;
            };
        };

        QUIC_CREDENTIAL_CONFIG_HELPER config;
        memset(&config, 0, sizeof(config));
        if(is_server){
#ifdef WIN32
            config.CredConfig.Flags = QUIC_CREDENTIAL_FLAG_NONE;
            config.CredConfig.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_HASH;
            config.CredConfig.CertificateHash = &config.CertHash;
            parse_hash_dat((std::byte*)config.CredConfig.CertificateHash->ShaHash, hash_file);
#else
            config.CredConfig.Flags = QUIC_CREDENTIAL_FLAG_NONE;
            config.CertFile.CertificateFile = cert;
            config.CertFile.PrivateKeyFile = private_key;
            config.CredConfig.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE;
#endif
            config.CredConfig.CertificateFile = &config.CertFile;
        }else{
            config.CredConfig.Type = QUIC_CREDENTIAL_TYPE_NONE;
            config.CredConfig.Flags = QUIC_CREDENTIAL_FLAG_CLIENT | QUIC_CREDENTIAL_FLAG_NO_CERTIFICATE_VALIDATION;
        }

        if(auto status = MsQuic->ConfigurationOpen(registration, &_alpn, 1, &settings, sizeof(settings), nullptr, &configuration); QUIC_FAILED(status))
        {
            std::cout << "Error at ConfigurationLoadCredential: " << status << std::endl;
            exit(1);
        }
        if(auto status = MsQuic->ConfigurationLoadCredential(configuration, &config.CredConfig); QUIC_FAILED(status))
        {
            std::cout << "Error at ConfigurationLoadCredential: " << status << std::endl;
            exit(1);
        }
    }

}
