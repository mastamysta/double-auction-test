#pragma once

#include "client.hpp"
#include "book_order_proto.hpp"

namespace exchange
{

class ExchangeClient
{
private:
    using PacketType = order_protocol::GenericMessage;
    using ClientType = UDSClient<PacketType>;
    
public:
    auto send_order(const PacketType& packet)
    {
        auto client = ClientType{};
        return client.send_msg_and_get_response(packet);
    }
};
    
}
