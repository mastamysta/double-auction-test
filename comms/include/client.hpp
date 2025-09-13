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
 
template <typename T>
requires std::is_trivial_v<T>
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

    auto send_msg(const T& data) const -> std::expected<void, SocketError>
    {
        auto addr = sockaddr_un{};
        memset(&addr, 0, sizeof(struct sockaddr_un));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, SOCKET_PATH, sizeof(SOCKET_PATH) - 1);

        if (connect(m_socket, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1)
        {
            std::cout << std::format("Failed to connect socket. {}\n", errno);
            return std::unexpected(SocketError::ConnectFailed);
        }

        if (send(m_socket, reinterpret_cast<const char*>(&data), sizeof(T), 0) == -1)
        {
            std::cout << std::format("Failed to write to socket {}\n", errno);
            return std::unexpected(SocketError::SendFailed);
        }

        return {};
    }

private:
    const char *SOCKET_PATH = "foobar";
    static constexpr int DEFAULT_PROTOCOL = 0;

    int m_socket;

};
   
}
