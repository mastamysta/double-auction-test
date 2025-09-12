#include <iostream>
#include <format>
#include <functional>

#include "server.hpp"
#include "book_order_proto.hpp"
#include "book.hpp"

using namespace order_protocol;

class ExchangeServer
{
public:
    ExchangeServer()
    {
        auto handler_wrapper = [&](auto message){
            this->handle_message(message);
        };
        m_server.post_on_recv_callback(handler_wrapper);   
        m_server.start_server();    
    }

private:
    auto handle_message(GenericMessage msg) -> void
    {
        switch (msg.message_type)
        {
        case MessageTypeID::LIMIT:
            if (msg.details.lim.side == Side::BUY)
                m_book.limit_buy(msg.details.lim.volume,
                                 msg.details.lim.price);
            else
                m_book.limit_sell(msg.details.lim.volume,
                                 msg.details.lim.price);

            break;

        case MessageTypeID::FOK:
            if (msg.details.fok.side == Side::BUY)
                m_book.fok_buy(msg.details.lim.volume,
                                 msg.details.lim.price);
            else
                m_book.fok_sell(msg.details.lim.volume,
                                 msg.details.lim.price);

            break;

        case MessageTypeID::CANCEL:
            m_book.cancel_order(msg.details.can.order_id);

            break;

        default:
            // Under normal execution we would not want to throw here
            // as we don't want to terminate the exchange for any invalid
            // order that is sent in, but useful for debug.
            throw std::runtime_error("Unrecognized message type sent to exchange.");
        }
    }

    UDSServer<GenericMessage> m_server;
    Book m_book;
};

int main(int argc, const char *argv[])
{
    auto server = ExchangeServer{};

    return 0;
}
