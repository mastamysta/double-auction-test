#pragma once

#include <cstdint>
#include <functional>
#include <queue>
#include <set>

#include <uuid/uuid.h>

using order_id = uint32_t;
using order_size = uint32_t;
using order_price = uint32_t;
using order_complete_cb = std::function<int(order_id, order_size, order_price)>;

enum class order_type
{
    LIM_BUY,
    LIM_SELL
};

struct order
{
    order_id id;
    order_size size;
    order_price price;
    order_type type;
};

class book
{
public:

    auto limit_buy(order_size, order_price) -> order_id;

    auto limit_sell(order_size, order_price) -> order_id;

    auto fok_buy(order_size, order_price) -> order_id; // UNSUPPORTED

    auto fok_sell(order_size, order_price) -> order_id; // UNSUPPORTED

    auto cancel_order(order_id id) -> void;

    auto post_order_complete_callback(order_complete_cb) -> void;

private:

    order_complete_cb _cb;
    std::set<order*, decltype([](order* lhs, order* rhs){ return lhs->price > rhs->price; })> buy_book;
    std::set<order*, decltype([](order* lhs, order* rhs){ return lhs->price < rhs->price; })> sell_book; // Take advantage of RB tree used to order set.
};

union uuid_hack
{
    uuid_t buf;
    order_id id;
};

auto build_order(order_type type, order_size size, order_price price) -> order*
{   
    auto o = new order;
    uuid_hack id;
    uuid_generate(id.buf);

    o->id = id.id;
    o->size = size;
    o->price = price;
    o->type = type;

    return o;
}

auto book::limit_buy(order_size size, order_price price) -> order_id
{
    while (size)
    {
        auto best_sell = *sell_book.begin();

        if (!best_sell)
            break;

        if (price >= best_sell->price)
        {
            if (size >= best_sell->size)
            {
                sell_book.erase(best_sell);
                size -= best_sell->size;
                _cb(best_sell->id, best_sell->size, best_sell->price);
                delete best_sell;
            }
            else
            {
                best_sell->size -= size;
                size = 0;
            }
        }
        else
            break;
    }

    if (!size)
        return -1;

    auto o = build_order(order_type::LIM_BUY, size, price);
    buy_book.insert(o);
    return o->id;  
}
auto book::limit_sell(order_size size, order_price price) -> order_id
{
    while (size)
    {
        auto best_buy = *buy_book.begin();

        if (!best_buy)
            break;

        if (price <= best_buy->price)
        {
            if (size >= best_buy->size)
            {
                sell_book.erase(best_buy);
                size -= best_buy->size;
                _cb(best_buy->id, best_buy->size, best_buy->price);
                delete best_buy;
            }
            else
            {
                best_buy->size -= size;
                size = 0;
            }
        }
        else
            break;
    }

    if (!size)
        return -1;

    auto o = build_order(order_type::LIM_SELL, size, price);
    sell_book.insert(o);
    return o->id;;
}
auto book::fok_buy(order_size, order_price) -> order_id
{
    // UNSUPPORTED
    return 0;   
}
auto book::fok_sell(order_size, order_price) -> order_id
{
    // UNSUPPORTED
    return 0;   
}

auto book::cancel_order(order_id id) -> void
{
    return;   
}

auto book::post_order_complete_callback(order_complete_cb cb) -> void
{
    _cb = cb;
    return;
}