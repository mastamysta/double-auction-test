#pragma once

// C
#include <sys/socket.h>
#include <errno.h>
#include <sys/un.h>
#include <unistd.h>

// C++
#include <format>
#include <iostream>
#include <expected>

#include "socket_ops.hpp"

namespace exchange
{
 
template <typename MessageType, typename ResponseType = MessageType>
requires std::is_trivial_v<MessageType> &&
            std::is_trivial_v<ResponseType>
class UDSClient
{
public:
    UDSClient()
    {
        if ((m_socket = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL)) == -1)
        {
            throw std::runtime_error(std::format("Failed to open socket {}\n", errno));
        }
    }

    UDSClient(const UDSClient& other) = delete;

    UDSClient operator=(const UDSClient& other) = delete;

    UDSClient(UDSClient&& other) = delete;

    UDSClient operator=(UDSClient&& other) = delete;

    ~UDSClient()
    {
        close(m_socket);
    }

    auto send_msg(const MessageType& data) const -> std::expected<void, SocketError>
    {
        if (auto ret = do_connect(m_socket, SOCKET_PATH))
        {}
        else
            return ret;

        if (auto ret = do_send(m_socket, data))
        {}
        else
            return ret;

        return {};
    }

    auto send_msg_and_get_response(const MessageType& data) const -> std::expected<ResponseType, SocketError>
    {
        if (auto ret = do_connect(m_socket, SOCKET_PATH))
        {}
        else
            return std::unexpected{ret.error()};

        if (auto ret = do_send(m_socket, data))
        {}
        else
            return std::unexpected{ret.error()};

        auto response = ResponseType{};

        if (auto ret = do_recv(m_socket, response))
            return {response};
        else
            return std::unexpected{ret.error()};

    }

private:
    const char *SOCKET_PATH = "foobar";
    static constexpr int DEFAULT_PROTOCOL = 0;

    int m_socket;

};
   
}
