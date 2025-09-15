#include "patient_agent.hpp"
#include "exchange_client.hpp"
#include "book_order_proto.hpp"

namespace exchange
{

class PatientExchangeAgent
{
public:
    PatientExchangeAgent():
    m_agent(order_placement_rate,
            order_cancellation_rate,
            order_size)
    {
        m_agent.post_place_callback(
        [&](std::size_t order_size, std::size_t order_price){
            return this->place_callback(order_size, order_price);
        });
        m_agent.post_cancel_callback(
        [&](auto order_id){
            return this->cancel_callback(order_id);
        });
    }

    auto act()
    {
        m_agent.act();
    }
private:
    // 0.4 per timestep
    static constexpr double order_placement_rate = 0.4;
    static constexpr double order_cancellation_rate = order_placement_rate;
    static constexpr std::size_t order_size = 10;

    PatientAgent m_agent;
    ExchangeClient m_client;

    std::function<bool(PatientAgent::OrderIDType)> cancel_callback_wrapper;
    std::function<PatientAgent::OrderIDType(std::size_t, std::size_t)> place_callback_wrapper;

    bool cancel_callback(PatientAgent::OrderIDType order_id)
    {
        auto packet = order_protocol::GenericMessage{};
        packet.message_type = order_protocol::MessageTypeID::CANCEL;
        packet.details.can = order_protocol::CancelDetails{order_id};

        if (auto ret = m_client.send_order(packet))
            // This is a slightly odd one. If cancellation failed
            // we probably want the user to go away and see if 
            // their order filled on their own.
            return ret.value().details.cresp.cancelled;
        else
            return false;
    }

    PatientAgent::OrderIDType place_callback(std::size_t order_size, std::size_t order_price)
    {
        auto packet = order_protocol::GenericMessage{};
        packet.message_type = order_protocol::MessageTypeID::LIMIT;
        packet.details.lim = 
            order_protocol::LimitDetails{order_price, 
                                         order_size,
                                         order_protocol::Side::BUY};

        if (auto ret = m_client.send_order(packet)) 
        {
            auto response = ret.value();

            if (response.message_type != order_protocol::MessageTypeID::LIM_RESP)
                std::cout << "Somehow we got the wrong response type.\n";

            if (response.details.lresp.filled)
                return -2;

            return response.details.lresp.order_id;
        }
        else
        {
            std::cout << "Send error!\n";
            return -1;
        }
    }
};

}

using namespace exchange;

int main(int argc, const char *argv[])
{
    auto agent = PatientExchangeAgent{};
    agent.act();

    return 0;
}
