#pragma once

#include <cstdint>
#include <functional>

using order_id = uint32_t;
using order_size = uint32_t;
using order_price = uint32_t;

class book
{
public:

    auto limit_buy(order_size, order_price) -> order_id;

    auto limit_sell(order_size, order_price) -> order_id;

    auto fok_buy(order_size, order_price) -> order_id; // UNSUPPORTED

    auto fok_sell(order_size, order_price) -> order_id; // UNSUPPORTED

    auto cancel_order(order_id id) -> void;

    auto post_order_complete_callback(std::function<int(order_id, order_size, order_price)>) -> void;
};

auto book::limit_buy(order_size, order_price) -> order_id
{
    return 0;   
}
auto book::limit_sell(order_size, order_price) -> order_id
{
    return 0;   
}
auto book::fok_buy(order_size, order_price) -> order_id
{
    return 0;   
}
auto book::fok_sell(order_size, order_price) -> order_id
{
    return 0;   
}
auto book::cancel_order(order_id id) -> void
{
    return;   
}
auto book::post_order_complete_callback(std::function<int(order_id, order_size, order_price)>) -> void
{
    return;
}