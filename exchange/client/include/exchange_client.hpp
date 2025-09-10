#pragma once

#include "client.hpp"
#include "character_buffer.hpp"

class ExchangeClient
{
private:
    using PacketType = StringBufferWithMetaData;
    using ClientType = UDSClient<PacketType>;

public:
    auto send_order(const PacketType& packet)
    {
        auto client = ClientType{};
        return client.send_msg(packet);
    }
};
