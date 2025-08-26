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

#include "book.hpp"
#include "character_buffer.hpp"

template <typename T>
requires std::is_trivial_v<T>
class UDSBookServer
{
public:
    enum class ListenError
    {
        AcceptFailed,
        RecvFailed
    };

    UDSBookServer()
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
    UDSBookServer(const UDSBookServer& other) = delete;
    UDSBookServer operator=(const UDSBookServer& other) = delete;

    UDSBookServer(UDSBookServer&& other) = delete;
    UDSBookServer operator=(UDSBookServer&& other) = delete;

    ~UDSBookServer()
    {
        close(m_socket);
    }

    auto post_on_recv_callback(std::function<void(const T&)> recv_callback)
    {
        m_recv_callback = recv_callback;
    }

    auto start_server() const -> std::expected<void, ListenError>
    {
        auto ret = std::expected<void, ListenError>{};

        while(ret = wait_msg()) { ; }

        return ret;
    }

private:
    static constexpr int MAX_QUEUE_LEN = 128;
    const char *SOCKET_PATH = "foobar";
    static constexpr int DEFAULT_PROTOCOL = 0;

    int m_socket;
    std::function<void(const T&)> m_recv_callback;

    auto wait_msg() const -> std::expected<void, ListenError>
    {
        struct sockaddr peer_addr;
        socklen_t other_addrlen;
        int newsock;

        if ((newsock = accept(m_socket, &peer_addr, &other_addrlen)) == -1)
        {
            std::cout << std::format("Unable to accept on socket. Errno: {}\n", errno);
            return std::unexpected{ListenError::AcceptFailed};
        }

        auto received_object = T{};

        if (recv(newsock, &received_object, sizeof(T), 0) == -1)
        {
            std::cout << std::format("Unable to read socket. Errno: {}\n", errno);
            return std::unexpected{ListenError::RecvFailed};
        }

        if (m_recv_callback)
            m_recv_callback(received_object);

        close(newsock);

        return {};
    }

};

template<>
struct std::formatter<UDSBookServer<StringBufferWithMetaData>::ListenError>
{
    constexpr auto parse(std::format_parse_context& context)
    {
        return context.begin();
    }

    auto format(const UDSBookServer<StringBufferWithMetaData>::ListenError& err, std::format_context& context) const
    {
        return std::format_to(context.out(), "{}", "Some_fooey_error");
    }
};

int main(int argc, const char *argv[])
{
    auto server = UDSBookServer<StringBufferWithMetaData>{};

    auto on_recv_callback = [](auto data){
        std::cout << std::format("Protocol: {}\nMessage: {}\nClient ID: {}\n",
                                 data.protocol_id,
                                 data.buffer,
                                 data.client_id);
    };
    server.post_on_recv_callback(on_recv_callback);

    if (auto ret = server.start_server()) {}
    else
    {
        std::cout << std::format("wait_msg() failed. {}\n", ret.error());
    }

    return 0;
}
