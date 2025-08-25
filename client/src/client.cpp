// C
#include <sys/socket.h>
#include <errno.h>
#include <sys/un.h>
#include <unistd.h>

// C++
#include <format>
#include <iostream>
#include <expected>


class UDSClient
{
public:
    enum class SendError
    {
        ConnectFailed,
        SendFailed
    };

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

    template <size_t BUFFER_LENGTH>
    auto send_msg(std::array<char, BUFFER_LENGTH> buffer) const -> std::expected<void, SendError>
    {
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(struct sockaddr_un));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, SOCKET_PATH.c_str(), sizeof(SOCKET_PATH.length()) - 1);

        if (connect(m_socket, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1)
        {
            std::cout << std::format("Failed to connect socket. {}\n", errno);
            return std::unexpected(SendError::ConnectFailed);
        }

        if (send(m_socket, buffer.data(), BUFFER_LENGTH+1, 0) == -1)
        {
            std::cout << std::format("Failed to write to socket {}\n", errno);
            return std::unexpected(SendError::SendFailed);
        }

        return {};
    }

private:
    static constexpr std::string SOCKET_PATH = "foobar";
    static constexpr int DEFAULT_PROTOCOL = 0;

    int m_socket;

};

int main(int argc, const char *argv[])
{
    UDSClient client{};

    auto message = "Whats up you nerd.";
    std::array<char, 255> buffer{};
    strncpy(buffer.data(), message, strlen(message)+1);

    if (auto ret = client.send_msg(buffer)) {}
    else
    {
        std::cout << "No info gathered for send failure.\n";
    }

    return 0;
}
