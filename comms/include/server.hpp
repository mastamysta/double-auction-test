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

#include "socket_ops.hpp"

namespace exchange
{

template <typename T>
requires std::is_trivial_v<T>
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
        close(m_socket);
    }

    auto post_on_recv_callback(std::function<void(const T&)> recv_callback)
    {
        m_recv_callback = recv_callback;
    }

    auto start_server() const -> std::expected<void, SocketError>
    {
        auto ret = std::expected<void, SocketError>{};

        while(ret = wait_msg()) { ; }

        return ret;
    }

private:
    static constexpr int MAX_QUEUE_LEN = 128;
    const char *SOCKET_PATH = "foobar";
    static constexpr int DEFAULT_PROTOCOL = 0;

    int m_socket;
    std::function<void(const T&)> m_recv_callback;

    auto wait_msg() const -> std::expected<void, SocketError>
    {
        auto peer_addr = sockaddr{};
        auto other_addrlen = socklen_t{};
        auto newsock{0};

        if ((newsock = accept(m_socket, &peer_addr, &other_addrlen)) == -1)
        {
            std::cout << std::format("Unable to accept on socket. Errno: {}\n", errno);
            return std::unexpected{SocketError::AcceptFailed};
        }

        auto received_object = T{};

        if (recv(newsock, &received_object, sizeof(T), 0) == -1)
        {
            std::cout << std::format("Unable to read socket. Errno: {}\n", errno);
            return std::unexpected{SocketError::RecvFailed};
        }

        if (m_recv_callback)
            m_recv_callback(received_object);

        close(newsock);

        return {};
    }

};

}
