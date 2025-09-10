#include <iostream>

#include "exchange_client.hpp"
#include "character_buffer.hpp"

int main(int argc, const char *argv[])
{
    auto client = ExchangeClient{};
    auto packet = StringBufferWithMetaData{};

    packet.client_id = 21;
    const auto *protocol_id = "FIX";
    const auto *buffer = "HELLO WORLD MY NAME IS BEN.";
    strncpy(packet.protocol_id, protocol_id, strlen(protocol_id));
    strncpy(packet.buffer, buffer, strlen(buffer));

    if (auto ret = client.send_order(packet)) 
    {}
    else
    {
        std::cout << "Send error!\n";
    }

    std::cout << "Foo\n";
    return 0;
}
