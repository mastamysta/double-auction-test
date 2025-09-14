#pragma once

// C
#include <sys/socket.h>
#include <errno.h>
#include <sys/un.h>
#include <unistd.h>
#include <poll.h>

// C++
#include <iostream>
#include <format>
#include <expected>
#include <string>
#include <type_traits>
#include <functional>
#include <csignal>

#include "socket_ops.hpp"

namespace exchange
{

template <typename MessageType, typename ResponseType = MessageType>
requires std::is_trivial_v<MessageType> && 
            std::is_trivial_v<ResponseType>
class UDSServer
{
public:
    UDSServer()
    {
        m_socket = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);

        if (m_socket == -1)
        {
            throw std::runtime_error(std::format("Unable to open socket. Errno: {}\n", errno));
        }

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(struct sockaddr_un));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, SOCKET_PATH, sizeof(SOCKET_PATH) - 1);

        unlink(SOCKET_PATH);

        if (bind(m_socket, 
                (struct sockaddr *)&addr,
                sizeof(struct sockaddr_un)) == -1)
        {
            throw std::runtime_error(std::format("Unable to bind to address. Errno: {}\n", errno));
        }

        if (listen(m_socket, MAX_QUEUE_LEN) == -1)
        {
            throw std::runtime_error(std::format("Unable to listen to socket. Errno: {}\n", errno));
        }
    }

    // Taking a copy of a file descriptor is not meaningful.
    UDSServer(const UDSServer& other) = delete;
    UDSServer operator=(const UDSServer& other) = delete;

    UDSServer(UDSServer&& other) = delete;
    UDSServer operator=(UDSServer&& other) = delete;

    ~UDSServer()
    {
        std::cout << "DTor.\n";
        close(m_socket);
    }

    auto post_on_recv_callback(std::function<void(const MessageType&)> recv_callback)
    {
        m_recv_callback = recv_callback;
    }

    auto post_response_gen_callback(std::function<ResponseType(const MessageType&)> resp_callback)
    {
        m_response_gen_callback = resp_callback;
    }

    auto start_server() const -> std::expected<void, SocketError>
    {
        auto ret = std::expected<void, SocketError>{};

        while(ret = wait_msg_and_respond()) { ; }

        return std::unexpected(ret.error());
    }

private:
    static constexpr int MAX_QUEUE_LEN = 128;
    const char *SOCKET_PATH = "foobar";
    static constexpr int DEFAULT_PROTOCOL = 0;

    int m_socket;
    std::function<void(const MessageType&)> m_recv_callback;
    std::function<ResponseType(const MessageType&)> m_response_gen_callback;

    auto wait_msg() const -> std::expected<void, SocketError>
    {
        auto incoming_socket{-1};

        if (auto ret = do_accept(m_socket))
        {
            incoming_socket = ret.value();
        }
        else
            return std::unexpected(ret.error());

        auto received_object = MessageType{};

        if (auto ret = do_recv(incoming_socket, received_object))
        {}
        else
            return ret;
        

        if (m_recv_callback)
            m_recv_callback(received_object);

        close(incoming_socket);

        return {};
    }

    auto wait_msg_and_respond() const -> std::expected<void, SocketError>
    {
        auto incoming_socket{-1};

        if (auto ret = do_accept(m_socket))
        {
            incoming_socket = ret.value();
        }
        else
            return std::unexpected(ret.error());

        auto received_object = MessageType{};

        if (auto ret = do_recv(incoming_socket, received_object))
        {}
        else
            return ret;

        auto response = ResponseType{};

        if (m_response_gen_callback)
            response = m_response_gen_callback(received_object);

        if (auto ret = do_send(incoming_socket, response))
        {}
        else
            return ret;

        close(incoming_socket);

        return {};
    }
};

}
