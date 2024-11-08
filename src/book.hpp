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

    auto cancel_order(order_id id) -> bool;

    auto post_order_complete_callback(order_complete_cb) -> void;

private:

    order_complete_cb _cb;
    std::set<order*, decltype([](order* lhs, order* rhs){ return lhs->price > rhs->price; })> buy_book;
    std::set<order*, decltype([](order* lhs, order* rhs){ return lhs->price < rhs->price; })> sell_book; // Take advantage of RB tree used to order set.
    std::map<order_id, order*> order_list;

    template <order_type OrderType>
    requires (OrderType == order_type::LIM_BUY)
    auto& get_order_book()
    {
        return buy_book;
    }

    template <order_type OrderType>
    requires (OrderType == order_type::LIM_SELL)
    auto& get_order_book()
    {
        return sell_book;
    }

    template <order_type OrderType>
    requires (OrderType == order_type::LIM_BUY)
    auto& get_opposing_order_book()
    {
        return sell_book;
    }

    template <order_type OrderType>
    requires (OrderType == order_type::LIM_SELL)
    auto& get_opposing_order_book()
    {
        return buy_book;
    }

    template <order_type OrderType>
    auto common_add_order(order_size, order_price) -> order_size;
};

union uuid_hack
{
    uuid_t buf;
    order_id id;
};

template <order_type OrderType>
auto build_order(order_size size, order_price price) -> order*
{   
    auto o = new order;
    uuid_hack id;
    uuid_generate(id.buf);

    o->id = id.id;
    o->size = size;
    o->price = price;
    o->type = OrderType;

    return o;
}

template <order_type OrderType>
requires (OrderType == order_type::LIM_BUY)
constexpr auto get_is_better()
{
    return [](order_price buy, order_price sell) { return buy >= sell; };
}

template <order_type OrderType>
requires (OrderType == order_type::LIM_SELL)
constexpr auto get_is_better()
{
    return [](order_price sell, order_price buy) { return sell <= buy; };
}

template <order_type OrderType>
auto book::common_add_order(order_size size, order_price price) -> order_id
{
    order* best;
    auto opposing_book = get_opposing_order_book<OrderType>();
    constexpr auto better = get_is_better<OrderType>();

    while (size)
    {
        auto best = *(opposing_book.begin());

        if (!best)
            break;

        if (better(price, best->price))
        {
            if (size >= best->size)
            {
                opposing_book.erase(best);
                order_list.erase(best->id);
                size -= best->size;
                _cb(best->id, best->size, best->price);
                delete best;
            }
            else
            {
                _cb(best->id, best->size - size, best->price);
                best->size -= size;
                size = 0;
            }
        }
        else
            break;
    }
    
    if (!size)
        return -1;

    auto& same_book = get_order_book<OrderType>();
    auto o = build_order<OrderType>(size, price);
    same_book.insert(o);
    order_list[o->id] = o;
    return o->id;  
}

auto book::limit_buy(order_size size, order_price price) -> order_id
{
    return common_add_order<order_type::LIM_BUY>(size, price);
}
auto book::limit_sell(order_size size, order_price price) -> order_id
{
    return common_add_order<order_type::LIM_SELL>(size, price);
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

auto book::cancel_order(order_id id) -> bool
{
    if (order_list.find(id) == order_list.end())
        return false;

    auto o = order_list[id];
    order_list.erase(id);

    if (o->type == order_type::LIM_BUY)
        buy_book.erase(o);
    else
        sell_book.erase(o);

    delete o;

    return true;   
}

auto book::post_order_complete_callback(order_complete_cb cb) -> void
{
    _cb = cb;
    return;
}