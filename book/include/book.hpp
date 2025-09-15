#pragma once

#include <cstdint>
#include <functional>
#include <queue>
#include <set>
#include <map>
#include <format>
#include <iostream>

#include <uuid/uuid.h>

using order_id = uint32_t;
using order_size = uint32_t;
using order_price = uint32_t;
using order_complete_cb = std::function<int(order_id, order_size, order_price)>;

enum class order_type
{
    LIM_BUY,
    LIM_SELL,
    FOK_BUY,
    FOK_SELL
};

struct order
{
    order_id id;
    order_size size;
    order_price price;
    order_type type;
};

inline auto better_buy = [](order* lhs, order* rhs){ return lhs->price > rhs->price; };
inline auto better_sell = [](order* lhs, order* rhs){ return lhs->price < rhs->price; };

class Book
{
public:
    using OrderIDType = order_id;

    auto limit_buy(order_size, order_price) -> OrderIDType;

    auto limit_sell(order_size, order_price) -> OrderIDType;

    auto fok_buy(order_size, order_price) -> bool;

    auto fok_sell(order_size, order_price) -> bool;

    auto cancel_order(OrderIDType id) -> bool;

    auto post_order_complete_callback(order_complete_cb) -> void;

private:

    order_complete_cb _cb;
    std::set<order*, decltype(better_buy)> buy_book;
    std::set<order*, decltype(better_sell)> sell_book; // Take advantage of RB tree used to order set.
    std::map<OrderIDType, order*> order_list;

    template <order_type OrderType>
    requires (OrderType == order_type::LIM_BUY || OrderType == order_type::FOK_BUY)
    auto& get_order_book()
    {
        return buy_book;
    }

    template <order_type OrderType>
    requires (OrderType == order_type::LIM_SELL || OrderType == order_type::FOK_SELL)
    auto& get_order_book()
    {
        return sell_book;
    }

    template <order_type OrderType>
    requires (OrderType == order_type::LIM_BUY || OrderType == order_type::FOK_BUY)
    auto& get_opposing_order_book()
    {
        return sell_book;
    }

    template <order_type OrderType>
    requires (OrderType == order_type::LIM_SELL || OrderType == order_type::FOK_SELL)
    auto& get_opposing_order_book()
    {
        return buy_book;
    }

    template <order_type OrderType>
    auto common_add_order(order_size, order_price) -> order_size;

    template <order_type OrderType>
    auto common_fok_order(order_size, order_price) -> bool;

    template <order_type OrderType>
    auto action(order_size, order_price, bool);

};

union uuid_hack
{
    uuid_t buf;
    order_id id;
};

template <order_type OrderType>
auto inline build_order(order_size size, order_price price) -> order*
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
requires (OrderType == order_type::LIM_BUY || OrderType == order_type::FOK_BUY)
constexpr inline auto get_is_better()
{
    return [](order_price buy, order_price sell) { return buy >= sell; };
}

template <order_type OrderType>
requires (OrderType == order_type::LIM_SELL || OrderType == order_type::FOK_SELL)
constexpr inline auto get_is_better()
{
    return [](order_price sell, order_price buy) { return sell <= buy; };
}

template <order_type OrderType>
inline auto Book::action(order_size size, order_price price, bool dry_run)
{
    auto& opposing_book = get_opposing_order_book<OrderType>();
    constexpr auto better = get_is_better<OrderType>();
    bool erase_last = true;
    auto best = opposing_book.begin();

    while (size)
    {
        if (best == opposing_book.end())
            break;

        if (better(price, (*best)->price))
        {
            if (size >= (*best)->size)
            {
                size -= (*best)->size;

                if (!dry_run)
                {
                    if (_cb)
                        _cb((*best)->id, (*best)->size, (*best)->price);

                    order_list.erase((*best)->id);
                    delete *best;
                }
            }
            else
            {
                if (!dry_run)
                {
                    if (_cb)
                        _cb((*best)->id, (*best)->size - size, (*best)->price);
                    
                    (*best)->size -= size;
                }

                size = 0;
                best++;
                break;
            }
        }
        else
            break;

        best++;
    }

    if (!dry_run)
    {
        opposing_book.erase(opposing_book.begin(), best);
    }

    return size;
}

template <order_type OrderType>
inline auto Book::common_add_order(order_size size, order_price price) -> OrderIDType
{
    auto remaining_size = action<OrderType>(size, price, false);
    
    if (!remaining_size)
        return -1;

    auto& same_book = get_order_book<OrderType>();
    auto o = build_order<OrderType>(remaining_size, price);
    same_book.insert(o);
    order_list[o->id] = o;
    return o->id;  
}

template <order_type OrderType>
inline auto Book::common_fok_order(order_size size, order_price price) -> bool
{
    auto remaining_size = action<OrderType>(size, price, true);
    
    if (remaining_size)
        return false;

    action<OrderType>(size, price, false);
    return true;
}

inline auto Book::limit_buy(order_size size, order_price price) -> OrderIDType
{
    return common_add_order<order_type::LIM_BUY>(size, price);
}
inline auto Book::limit_sell(order_size size, order_price price) -> OrderIDType
{
    return common_add_order<order_type::LIM_SELL>(size, price);
}
inline auto Book::fok_buy(order_size size, order_price price) -> bool
{
    return common_fok_order<order_type::FOK_BUY>(size, price);
}
inline auto Book::fok_sell(order_size size, order_price price) -> bool
{
    return common_fok_order<order_type::FOK_SELL>(size, price);
}

inline auto Book::cancel_order(OrderIDType id) -> bool
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

inline auto Book::post_order_complete_callback(order_complete_cb cb) -> void
{
    _cb = cb;
    return;
}
