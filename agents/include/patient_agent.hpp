#pragma once

#include <random>
#include <vector>
#include <ranges>
#include <functional>
#include <cmath>

namespace agents
{

class PatientAgent
{
public:
    PatientAgent(double order_placement_rate, 
                 double order_cancellation_rate,
                 std::size_t order_size,
                 unsigned money,
                 std::function<bool(OrderIDType)> cancel_callback,
                 std::function<OrderIDType(std::size_t, std::size_t)> place_callback): 
        m_placement_distribution({order_placement_rate}),
        m_cancellation_distribution({order_cancellation_rate}),
        m_order_size(order_size),
        m_money(money),
        m_cancel_callback(cancel_callback),
        m_place_callback(place_callback)
    {
        auto rd = std::random_device{};
        auto seed_data = std::array<SeedType, RandomGeneratorType::state_size>;
        std::generate(std::begin(seed_data), std::end(seed_data), std::ref(seed_data));
        m_eng = RandomGeneratorType{seed_data};
    }

    auto act()
    {
        try_cancel_order();
        try_place_order();
    }

private:
    using SeedType = int;
    using RandomGeneratorType = std::mt19937;
    using OrderIDType = std::size_t;

    RandomGeneratorType m_eng;
    std::poisson_distribution<unsigned> m_placement_distribution;
    std::poisson_distribution<unsigned> m_cancellation_distribution;
    unsigned m_order_size;
    unsigned m_money;

    std::vector<OrderIDType> m_active_orders;
    std::function<bool(OrderIDType)> m_cancel_callback;
    std::function<OrderIDType(std::size_t, std::size_t)> place_callback;

    auto try_cancel_order()
    {
        auto orders_to_cancel = m_cancellation_distribution(m_eng);

        for (auto _: std::ranges::views::iota(0, orders_to_cancel))
        {
            auto index_dist =
                 std::uniform_int_distribution<unsigned>{0, m_active_orders.size()-1};

            auto index_to_cancel = index_dist(m_eng);
            auto success = m_cancel_callback(m_active_orders(index_to_cancel));

            if (success)
                m_active_orders.erase(index_to_cancel);
        }
    }

    auto try_place_order()
    {
        auto orders_to_place = m_placement_distribution(m_eng);

        for (auto _: std::ranges::views::iota(0, orders_to_place))
        {
            auto current_best = 0.0;
            auto price_dist = std::uniform_real_distribution(0, std::log(current_best));
            auto price = std::exp(price_dist(m_eng));

            auto order_id = m_place_callback(m_order_size, price);

            if (order_id != -1)
                m_active_orders.push_back(order_id);
        }
    }
};
    
}

