#include <iostream>
#include <string>
#include <memory>
#include <functional>

#include <cstring>

#include <fmt/core.h>

#include "quic.h"

namespace quic 
{
    struct OnError
    {
        template<class TFunc>
        OnError(const TFunc& func)
            : _succeeded(false)
            , _func(func)
        {
        }

        ~OnError()
        {
            if(_func && !_succeeded)
                _func();
        }

        void succeeded()
        {
            _succeeded = true;
        }

    private:
        bool _succeeded;
        std::function<void()> _func;
    };
    
    struct SendData
    {
        SendData(std::size_t size, const std::byte* data)
            : _buffer(std::make_unique<std::byte[]>(size))
            , _quic_buffer({.Length = static_cast<std::uint32_t>(size), .Buffer = reinterpret_cast<std::uint8_t*>(_buffer.get())})
        {
            std::memcpy(_buffer.get(), data, size);
        }

        SendData(const SendData&) = delete;
        SendData(SendData&& other) = delete;

        std::unique_ptr<std::byte[]> _buffer;
        QUIC_BUFFER _quic_buffer;
    };


    QUIC_STATUS client_connection_callback(HQUIC connection, void* context, QUIC_CONNECTION_EVENT* event);
    QUIC_STATUS client_stream_callback(HQUIC stream, void* context, QUIC_STREAM_EVENT* event);
    void client_send(HQUIC connection);

    QUIC_STATUS client_connection_callback(HQUIC connection, void* context, QUIC_CONNECTION_EVENT* event)
    {
        switch (event->Type) {
            case QUIC_CONNECTION_EVENT_CONNECTED:
                printf("[conn][%p] Connected\n", connection);
                client_send(connection);
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
            case QUIC_CONNECTION_EVENT_DATAGRAM_STATE_CHANGED:
                printf("[conn][%p] Datagram State Changed, enabled=%d, max_length=%hu\n", connection, event->DATAGRAM_STATE_CHANGED.SendEnabled, event->DATAGRAM_STATE_CHANGED.MaxSendLength);
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

    QUIC_STATUS client_stream_callback(HQUIC stream, void* context, QUIC_STREAM_EVENT* event)
    {
        switch (event->Type) {
        case QUIC_STREAM_EVENT_SEND_COMPLETE:
            delete (SendData*)(event->SEND_COMPLETE.ClientContext);
            printf("[strm][%p] Data sent\n", stream);
            break;
        case QUIC_STREAM_EVENT_RECEIVE:
            {
                printf("[strm][%p] Data received\n", stream);
                // printf("  data: %s\n", event->RECEIVE.Buffers);
            }
            break;
        case QUIC_STREAM_EVENT_PEER_SEND_ABORTED:
            printf("[strm][%p] Peer aborted\n", stream);
            break;
        case QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN:
            printf("[strm][%p] Peer shut down\n", stream);
            break;
        case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
            printf("[strm][%p] All done\n", stream);
            MsQuic->StreamClose(stream);
            break;
        default:
            break;
        }
        return QUIC_STATUS_SUCCESS;
    }

    void client_send(HQUIC connection)
    {
        OnError error{
            [&](){
                MsQuic->ConnectionShutdown(connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
            }
        };

        HQUIC stream = nullptr;

        if(auto status = MsQuic->StreamOpen(connection, QUIC_STREAM_OPEN_FLAG_NONE, client_stream_callback, nullptr, &stream); QUIC_FAILED(status))
        {
            std::cout << "Error at StreamOpen: " << status << std::endl;
            return;
        }

        std::cout << "[strm][" << stream << "] Stream opened." << std::endl;
        if(auto status = MsQuic->StreamStart(stream, QUIC_STREAM_START_FLAG_NONE); QUIC_FAILED(status))
        {
            std::cout << "Error at StreamStart : " << status << std::endl;
            return;
        }
        std::cout << "[strm][" << stream << "] Stream started." << std::endl;

        for(int i=0;i<5;i++){
            const char send_str[] = "abcdefghijklmnopqrstuvwxyz";

            auto send_data = std::make_unique<SendData>(sizeof(send_str), reinterpret_cast<const std::byte*>(send_str));

            std::cout << "[strm][" << stream << "] Sending data..." << std::endl;
            if(auto status = MsQuic->StreamSend(stream, &send_data->_quic_buffer, 1, QUIC_SEND_FLAG_FIN, send_data.get()); QUIC_FAILED(status))
            {
                std::cout << "Error at StreamSend: " << status << std::endl;
                return;
            }

            // 送信完了後にdeleteするのでunique_ptrで解放する責任は負わない
            send_data.release();
        }
        error.succeeded();
    }

    void as_client(const std::string& target, int port)
    {
#ifdef WIN32 
        init_configuration(_registration, _configuration, false, nullptr);
#else
        init_configuration(_registration, _configuration, false, nullptr, nullptr);
#endif

        HQUIC connection = nullptr;

        OnError error { [&](){ if(connection) { MsQuic->ConnectionClose(connection); }  } };

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

        puts("Press Enter to exit.\n\n");
        getchar();
    }

    // ============================================
    QUIC_STATUS server_listener_callback(HQUIC listener, void* context, QUIC_LISTENER_EVENT* event);
    QUIC_STATUS server_connection_callback(HQUIC listener, void* context, QUIC_CONNECTION_EVENT* event);
    QUIC_STATUS server_stream_callback(HQUIC stream, void* context, QUIC_STREAM_EVENT* event);

    QUIC_STATUS server_listener_callback(HQUIC listener, void* context, QUIC_LISTENER_EVENT* event)
    {
        switch(event->Type)
        {
        case QUIC_LISTENER_EVENT_NEW_CONNECTION:
            MsQuic->SetCallbackHandler(event->NEW_CONNECTION.Connection, (void*)server_connection_callback, nullptr);
            return MsQuic->ConnectionSetConfiguration(event->NEW_CONNECTION.Connection, _configuration);
        default:
            std::cout << "server_listener_callback: " << event->Type << std::endl;
            return QUIC_STATUS_NOT_SUPPORTED;
        }
    }

    QUIC_STATUS server_connection_callback(HQUIC connection, void* context, QUIC_CONNECTION_EVENT* event)
    {
        switch(event->Type)
        {
        case QUIC_CONNECTION_EVENT_CONNECTED:
            printf("[conn][%p] Connected\n", connection);
            MsQuic->ConnectionSendResumptionTicket(connection, QUIC_SEND_RESUMPTION_FLAG_NONE, 0, NULL);
            break;
        case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
            printf("[conn][%p] Shut down by transport, 0x%x\n", connection, event->SHUTDOWN_INITIATED_BY_TRANSPORT.Status);
            break;
        case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
            printf("[conn][%p] Shut down by peer, 0x%llu\n", connection, (unsigned long long)event->SHUTDOWN_INITIATED_BY_PEER.ErrorCode);
            break;
        case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
            printf("[conn][%p] All done\n", connection);
            MsQuic->ConnectionClose(connection);
            break;
        case QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED:
            printf("[strm][%p] Peer started\n", event->PEER_STREAM_STARTED.Stream);
            MsQuic->SetCallbackHandler(event->PEER_STREAM_STARTED.Stream, (void*)server_stream_callback, nullptr);
            break;
        case QUIC_CONNECTION_EVENT_RESUMED:
            printf("[conn][%p] Connection resumed!\n", connection);
            break;

        default:
            std::cout << "server_connection_callback: " << event->Type << std::endl;
        }
        return QUIC_STATUS_SUCCESS;
    }
    
    QUIC_STATUS server_stream_callback(HQUIC stream, void* context, QUIC_STREAM_EVENT* event)
    {
        switch (event->Type) {
        case QUIC_STREAM_EVENT_SEND_COMPLETE:
            free(event->SEND_COMPLETE.ClientContext);
            printf("[strm][%p] Data sent\n", stream);
            break;
        case QUIC_STREAM_EVENT_RECEIVE:
        {
            auto& recv = event->RECEIVE;
            printf("[strm][%p] Data received\n", stream);
            fmt::print(
                    "  offset : {}\n"
                    "  total_buf_length : {}\n"
                    "  buffer_cnt : {}\n"
                    "  flags : {}\n"
                    , recv.AbsoluteOffset
                    , recv.TotalBufferLength
                    , recv.BufferCount
                    , recv.Flags
                    );

            for(int i=0;i<recv.BufferCount;i++){
                fmt::print("  [{}] : {}\n", i, reinterpret_cast<const char*>(recv.Buffers[i].Buffer));
            }
        }
            break;
        case QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN:
            printf("[strm][%p] Peer shut down\n", stream);
            // ServerSend(Stream);
            break;
        case QUIC_STREAM_EVENT_PEER_SEND_ABORTED:
            printf("[strm][%p] Peer aborted\n", stream);
            MsQuic->StreamShutdown(stream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
            break;
        case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
            printf("[strm][%p] All done\n", stream);
            MsQuic->StreamClose(stream);
            break;
        default:
            break;
        }
        return QUIC_STATUS_SUCCESS;
    }

    void as_server(int port)
    {
#ifdef WIN32 
        init_configuration(_registration, _configuration, true, "cert/_wildcard.reanisz.info+3-sha.dat");
#else
        init_configuration(_registration, _configuration, true, "cert/server-crt.crt", "cert/server-privatekey.pem");
#endif

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
#ifndef WIN32
    dump(STATUS_PERMISSION_DENIED);
    dump(STATUS_EPOLL_ERROR);
    dump(STATUS_DNS_RESOLUTION_ERROR);
    dump(STATUS_SOCKET_ERROR);
#endif
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

