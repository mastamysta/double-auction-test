// C
#include <sys/socket.h>
#include <errno.h>
#include <sys/un.h>
#include <unistd.h>

// C++
#include <iostream>
#include <format>
#include <expected>
#include <string>

#include "book.hpp"

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
        strncpy(addr.sun_path, SOCKET_PATH.c_str(), sizeof(SOCKET_PATH.length()) - 1);

        unlink(SOCKET_PATH.c_str());

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

    UDSBookServer(const UDSBookServer& other) = delete;

    UDSBookServer operator=(const UDSBookServer& other) = delete;

    UDSBookServer(UDSBookServer&& other) = delete;

    UDSBookServer operator=(UDSBookServer&& other) = delete;

    ~UDSBookServer()
    {
        close(m_socket);
    }

    template <size_t BUFFER_LENGTH>
    auto wait_msg(std::array<char, BUFFER_LENGTH>& buffer) const -> std::expected<void, ListenError>
    {
        struct sockaddr peer_addr;
        socklen_t other_addrlen;
        int newsock;

        if ((newsock = accept(m_socket, &peer_addr, &other_addrlen)) == -1)
        {
            std::cout << std::format("Unable to accept on socket. Errno: {}\n", errno);
            return std::unexpected{ListenError::AcceptFailed};
        }

        if (recv(newsock, buffer.data(), BUFFER_LENGTH-1, 0) == -1)
        {
            std::cout << std::format("Unable to read socket. Errno: {}\n", errno);
            return std::unexpected{ListenError::RecvFailed};
        }

        close(newsock);

        std::cout << std::format("Message: {}\n", buffer.data());

        return {};
    }

private:
    static constexpr int MAX_QUEUE_LEN = 128;
    static constexpr std::string SOCKET_PATH = "foobar";
    static constexpr int DEFAULT_PROTOCOL = 0;
    int m_socket;
};

template<>
struct std::formatter<UDSBookServer::ListenError>
{
    constexpr auto parse(std::format_parse_context& context)
    {
        return context.begin();
    }

    auto format(const UDSBookServer::ListenError& err, std::format_context& context) const
    {
        return std::format_to(context.out(), "{}", "Some_fooey_error");
    }
};

int main(int argc, const char *argv[])
{
    UDSBookServer server{};

    std::array<char, 255> retbuf;
    
    if (auto ret = server.wait_msg(retbuf)) {}
    else
    {
        std::cout << std::format("wait_msg() failed. {}\n", ret.error());
    }

    return 0;
}
