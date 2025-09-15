#pragma once

#include <random>
#include <vector>
#include <ranges>
#include <functional>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <format>

namespace exchange
{

class PatientAgent
{
public:
    using OrderIDType = std::size_t;
    
    enum class PlaceOutcome
    {
        FILLED_IMMEDIATELY,
        FAILED
    };

    enum class Side: int
    {
        BUY = 0,
        SELL = 1
    };
    
    using PlaceOrderCallbackType = std::function<std::expected<OrderIDType, PlaceOutcome>(Side, std::size_t, std::size_t)>;

    PatientAgent(double order_placement_rate, 
                 double order_cancellation_rate,
                 std::size_t order_size): 
        m_placement_distribution({order_placement_rate}),
        m_cancellation_distribution({order_cancellation_rate}),
        m_order_size(order_size),
        m_buy_side_distribution(0, 1)
    {
        auto rd = std::random_device{};
        auto seed_data = std::array<SeedType, RandomGeneratorType::state_size>{};
        std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
        auto seed = std::seed_seq{seed_data.begin(), seed_data.end()};
        m_eng = RandomGeneratorType{seed};
    }

    auto act();

    auto post_place_callback(PlaceOrderCallbackType place_callback)
    {
        m_place_callback = place_callback;
    }

    auto post_cancel_callback(std::function<bool(OrderIDType)> cancel_callback)
    {
        m_cancel_callback = cancel_callback;
    }

private:
    using SeedType = int;
    using RandomGeneratorType = std::mt19937;

    RandomGeneratorType m_eng;
    std::poisson_distribution<unsigned> m_placement_distribution;
    std::poisson_distribution<unsigned> m_cancellation_distribution;
    std::uniform_int_distribution<unsigned> m_buy_side_distribution;
    unsigned m_order_size;

    std::vector<OrderIDType> m_active_orders;
    std::function<bool(OrderIDType)> m_cancel_callback;
    PlaceOrderCallbackType m_place_callback;

    auto try_cancel_order()
    {
        auto orders_to_cancel = m_cancellation_distribution(m_eng);

        if (orders_to_cancel > m_active_orders.size())
            orders_to_cancel = m_active_orders.size();

        for (auto _: std::views::iota(0u, orders_to_cancel))
        {
            auto index_dist =
                 std::uniform_int_distribution<unsigned long>{0, m_active_orders.size()};

            auto index_to_cancel = index_dist(m_eng);
            auto success = m_cancel_callback(m_active_orders[index_to_cancel]);

            if (success)
                m_active_orders.erase(m_active_orders.begin() + index_to_cancel);
            else
                // TODO: If not successful, was the order already filled?
                // For now, remove it all the same.
                m_active_orders.erase(m_active_orders.begin() + index_to_cancel);
        }
    }

    auto try_place_order() -> bool
    {
        auto orders_to_place = m_placement_distribution(m_eng);

        for (auto _: std::views::iota(0u, orders_to_place))
        {
            auto current_best = 45.0;
            auto price_dist = std::uniform_real_distribution<double>(0, std::log(current_best));
            auto price = std::exp(price_dist(m_eng));
            auto side = static_cast<Side>(m_buy_side_distribution(m_eng));

            if (auto ret = m_place_callback(side, m_order_size, price))
            {
                m_active_orders.push_back(ret.value());
            }
            else if (ret.error() == PlaceOutcome::FILLED_IMMEDIATELY)
            {
                // Don't add to the active order list, possibly use this
                // to tracl P/L metrics.
            }
            else
            {
                std::cout << std::format("Placement failed.");
                return false;
            }
        }

        return true;
    }
};
    
auto PatientAgent::act()
{
    try_cancel_order();
    return try_place_order();
}

}

