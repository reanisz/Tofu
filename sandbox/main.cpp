#include <iostream>
#include <string>

#include "quic.h"

namespace quic 
{
    QUIC_STATUS client_connection_callback(HQUIC connection, void* context, QUIC_CONNECTION_EVENT* event)
    {
        switch (event->Type) {
            case QUIC_CONNECTION_EVENT_CONNECTED:
                printf("[conn][%p] Connected\n", connection);
                // ClientSend(connection);
                break;
            case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
                printf("[conn][%p] Shut down by transport, 0x%x\n", connection, event->SHUTDOWN_INITIATED_BY_TRANSPORT.Status);
                break;
            case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
                printf("[conn][%p] Shut down by peer, 0x%llu\n", connection, (unsigned long long)event->SHUTDOWN_INITIATED_BY_PEER.ErrorCode);
                break;
            case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
                printf("[conn][%p] All done\n", connection);
                if (!event->SHUTDOWN_COMPLETE.AppCloseInProgress) {
                    MsQuic->ConnectionClose(connection);
                }
                break;
            case QUIC_CONNECTION_EVENT_RESUMPTION_TICKET_RECEIVED:
                printf("[conn][%p] Resumption ticket received (%u bytes):\n", connection, event->RESUMPTION_TICKET_RECEIVED.ResumptionTicketLength);
                for (uint32_t i = 0; i < event->RESUMPTION_TICKET_RECEIVED.ResumptionTicketLength; i++) {
                    printf("%.2X", (uint8_t)event->RESUMPTION_TICKET_RECEIVED.ResumptionTicket[i]);
                }
                printf("\n");
                break;
            default:
                std::cout << "client_connection_callback: " << event->Type << std::endl;
                break;
        }
        return QUIC_STATUS_SUCCESS;
    }

    void as_client(const std::string& target, int port)
    {
        init_configuration(_registration, _configuration, false, "cert/ca-crt.pem", "cert/ca-privatekey.pem");

        HQUIC connection = nullptr;

        if(auto status = MsQuic->ConnectionOpen(_registration, client_connection_callback, nullptr, &connection); QUIC_FAILED(status))
        {
            std::cout << "Error at ConnectionOpen : " << status << std::endl;
            return;
        }

        if(auto status = MsQuic->ConnectionStart(connection, _configuration, QUIC_ADDRESS_FAMILY_UNSPEC, target.c_str(), port); QUIC_FAILED(status))
        {
            std::cout << "Error at ConnectionStart : " << status << std::endl;
            return;
        }
    }

    QUIC_STATUS server_listener_callback(HQUIC listener, void* context, QUIC_LISTENER_EVENT* event)
    {
        switch(event->Type)
        {
        case QUIC_LISTENER_EVENT_NEW_CONNECTION:
        default:
            std::cout << "client_connection_callback: " << event->Type << std::endl;
            return QUIC_STATUS_NOT_SUPPORTED;
        }
    }

    void as_server(int port)
    {
        init_configuration(_registration, _configuration, true, "cert/server-crt.pem", "cert/server-privatekey.pem");

        HQUIC listener = nullptr;

        QUIC_ADDR address = {};

        QuicAddrSetFamily(&address, QUIC_ADDRESS_FAMILY_UNSPEC);
        QuicAddrSetPort(&address, port);

        if(auto status = MsQuic->ListenerOpen(_registration, server_listener_callback, nullptr, &listener); QUIC_FAILED(status))
        {
            std::cout << "Error at ListenerOpen: " << status << std::endl;
            return;
        }

        if(auto status = MsQuic->ListenerStart(listener, &_alpn, 1, &address); QUIC_FAILED(status))
        {
            std::cout << "Error at ListenerStart: " << status << std::endl;
            return;
        }

        puts("Press Enter to exit.\n\n");
        getchar();

    }

    void finalize()
    {
        if(MsQuic){
            if(_configuration)
            {
                MsQuic->ConfigurationClose(_configuration);
            }
            if(_registration)
            {
                MsQuic->RegistrationClose(_registration);
            }
            MsQuicClose(MsQuic);
        }
    }
}

void print_error_list()
{
#define dump(def) printf("QUIC_%s : %d\n", #def, QUIC_ ## def)
    dump(STATUS_SUCCESS);
    dump(STATUS_PENDING);
    dump(STATUS_CONTINUE);
    dump(STATUS_OUT_OF_MEMORY);
    dump(STATUS_INVALID_PARAMETER);
    dump(STATUS_INVALID_STATE);
    dump(STATUS_NOT_SUPPORTED);
    dump(STATUS_NOT_FOUND);
    dump(STATUS_BUFFER_TOO_SMALL);
    dump(STATUS_HANDSHAKE_FAILURE);
    dump(STATUS_ABORTED);
    dump(STATUS_ADDRESS_IN_USE);
    dump(STATUS_CONNECTION_TIMEOUT);
    dump(STATUS_CONNECTION_IDLE);
    dump(STATUS_INTERNAL_ERROR);
    dump(STATUS_CONNECTION_REFUSED);
    dump(STATUS_PROTOCOL_ERROR);
    dump(STATUS_VER_NEG_ERROR);
    dump(STATUS_UNREACHABLE);
    dump(STATUS_PERMISSION_DENIED);
    dump(STATUS_EPOLL_ERROR);
    dump(STATUS_DNS_RESOLUTION_ERROR);
    dump(STATUS_SOCKET_ERROR);
    dump(STATUS_TLS_ERROR);
    dump(STATUS_USER_CANCELED);
    dump(STATUS_ALPN_NEG_FAILURE);
#undef dump
}

int main(int argc, const char** argv)
{
    if(argc == 1)
    {
        std::cout << "Invalid Argument" << std::endl;
        return 1;
    }

    std::string command = argv[1];
    if(command == "print_error_list")
    {
        print_error_list();
    }
    else if(command == "client")
    {
        if(argc < 4)
        {
            std::cout << "Invalid Argument" << std::endl;
            return 1;
        }
        quic::init();
        auto target = argv[2];
        auto port = std::stoi(argv[3]);
        std::cout << "client mode to " << target << ":" << port << std::endl;
        quic::as_client(target, port);
    }
    else if(command == "server")
    {
        if(argc < 3)
        {
            std::cout << "Invalid Argument" << std::endl;
            return 1;
        }
        quic::init();
        auto port = std::stoi(argv[2]);
        std::cout << "server mode " << port << std::endl;
        quic::as_server(port);
    }

    std::cout << "." << std::endl;

    quic::finalize();

    return 0;
}

