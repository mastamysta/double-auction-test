#include <iostream>
#include <format>

#include "server.hpp"
#include "character_buffer.hpp"
#include "book.hpp"

class ExchangeServer
{
public:
    ExchangeServer()
    {
        m_server.start_server();    
        
        auto on_recv_callback = [](auto data){
        std::cout << std::format("Protocol: {}\nMessage: {}\nClient ID: {}\n",
                                 data.protocol_id,
                                 data.buffer,
                                 data.client_id);
        };
        m_server.post_on_recv_callback(on_recv_callback);
    }

private:
    UDSServer<StringBufferWithMetaData> m_server;
    Book m_book;
};

int main(int argc, const char *argv[])
{
    auto server = ExchangeServer{};

    return 0;
}
