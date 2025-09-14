#include <iostream>
#include <format>
#include <functional>

#include "server.hpp"
#include "book_order_proto.hpp"
#include "book.hpp"

using namespace order_protocol;
using namespace exchange;

class ExchangeServer
{
public:
    using BookType = Book;
    using OrderIDType = BookType::OrderIDType;

    ExchangeServer()
    {
        auto handler_wrapper = [&](auto message){
            return this->handle_message(message);
        };
        m_server.post_response_gen_callback(handler_wrapper);   
        m_server.start_server();    
    }

private:
    auto handle_message(GenericMessage msg) -> GenericMessage
    {
        auto response = GenericMessage{};

        switch (msg.message_type)
        {
        case MessageTypeID::LIMIT:
        {
            response.message_type = MessageTypeID::LIM_RESP;

            auto order_id = OrderIDType{};

            if (msg.details.lim.side == Side::BUY)
                order_id = m_book.limit_buy(msg.details.lim.volume,
                                            msg.details.lim.price);
            else
                order_id = m_book.limit_sell(msg.details.lim.volume,
                                            msg.details.lim.price);

            if (order_id == -1)
                response.details.lresp.filled = true;
            else
            {
                response.details.lresp.filled = false;
                response.details.lresp.order_id = order_id;
            }

            break;

        }
        case MessageTypeID::FOK:
        {
            response.message_type = MessageTypeID::FOK_RESP;
    
            auto filled = false;
    
            if (msg.details.fok.side == Side::BUY)
                filled = m_book.fok_buy(msg.details.lim.volume,
                                        msg.details.lim.price);
            else
                filled = m_book.fok_sell(msg.details.lim.volume,
                                         msg.details.lim.price);
    
            response.details.fresp.filled = filled;
    
            break;
        }
        case MessageTypeID::CANCEL:
        {
            response.message_type = MessageTypeID::CAN_RESP;

            response.details.cresp.cancelled =
                    m_book.cancel_order(msg.details.can.order_id);

            break;
        }
        default:
            // Under normal execution we would not want to throw here
            // as we don't want to terminate the exchange for any invalid
            // order that is sent in, but useful for debug.
            throw std::runtime_error("Unrecognized message type sent to exchange.");
        }

        return response;
    }

    UDSServer<GenericMessage> m_server;
    BookType m_book;
};

int main(int argc, const char *argv[])
{
    auto server = ExchangeServer{};

    return 0;
}
