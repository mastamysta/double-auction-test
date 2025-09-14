#include <iostream>

#include "exchange_client.hpp"
#include "book_order_proto.hpp"

using namespace order_protocol;
using namespace exchange;

int main(int argc, const char *argv[])
{
    auto client = ExchangeClient{};
    auto packet = GenericMessage{};

    packet.message_type = MessageTypeID::LIMIT;
    packet.details.lim = LimitDetails{ 21, 69, Side::BUY };

    if (auto ret = client.send_order(packet)) 
    {
        auto response = ret.value().message_type;

        std::cout << int(response == MessageTypeID::LIM_RESP) << std::endl;
    }
    else
    {
        std::cout << "Send error!\n";
    }

    return 0;
}
