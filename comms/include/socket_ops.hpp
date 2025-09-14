#pragma once

#include <sys/socket.h>

namespace exchange
{
    enum class SocketError
    {
        AcceptFailed,
        RecvFailed,
        ConnectFailed,
        SendFailed
    };

    auto do_connect(int socket,
                    const char *socket_path) -> std::expected<void, SocketError>
    {
        auto addr = sockaddr_un{};
        memset(&addr, 0, sizeof(struct sockaddr_un));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socket_path, strlen(socket_path));

        if (connect(socket, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1)
        {
            std::cout << std::format("Failed to connect socket. {}\n", errno);
            return std::unexpected(SocketError::ConnectFailed);
        }

        return {};
    }

    auto do_accept(const int& listen_socket) -> std::expected<int, SocketError>
    {
        sockaddr peer_addr;
        socklen_t peer_addrlen;

        auto sock = accept(listen_socket, &peer_addr, &peer_addrlen);

        if (sock == -1)
        {
            std::cout << std::format("Unable to accept on socket. Errno: {}\n", errno);
            return std::unexpected{SocketError::AcceptFailed};
        }
        else
        {
            return {sock};
        }
    }

    template <typename ReceivedType>
    auto do_recv(int socket,
                     ReceivedType& received_object) -> std::expected<void, SocketError>
    {
        if (recv(socket, &received_object, sizeof(ReceivedType), 0) == -1)
        {
            std::cout << std::format("Unable to read socket. Errno: {}\n", errno);
            return std::unexpected{SocketError::RecvFailed};
        }

        return {};
    }

    template <typename SentType>
    auto do_send(int socket,
                     const SentType& sent_object) -> std::expected<void, SocketError>
    {
        if (send(socket, reinterpret_cast<const char*>(&sent_object), sizeof(SentType), 0) == -1)
        {
            std::cout << std::format("Failed to write to socket {}\n", errno);
            return std::unexpected(SocketError::SendFailed);
        }

        return {};
    }
}

