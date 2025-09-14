#include <iostream>
#include <format>

#include "server.hpp"
#include "character_buffer.hpp"

using namespace exchange;

int main(int argc, const char *argv[])
{
    auto server = UDSServer<StringBufferWithMetaData>{};

    auto on_recv_callback = [](auto data){
        std::cout << std::format("Protocol: {}\nMessage: {}\nClient ID: {}\n",
                                 data.protocol_id,
                                 data.buffer,
                                 data.client_id);
        return data;
    };
    server.post_response_gen_callback(on_recv_callback);

    if (auto ret = server.start_server()) {}
    else
    {
        std::cout << std::format("wait_msg() failed. \n");
    }

    return 0;
}
