#include <iostream>
#include <tuple>

#include "gtest/gtest.h"

#include "book.hpp"

TEST(book_tests, can_instantiate)
{
    book b;
}

TEST(book_tests, can_buy)
{
    book b;

    b.limit_buy(100, 100);
}

TEST(book_tests, can_sell)
{
    book b;

    b.limit_sell(100, 100);
}

TEST(book_tests, can_match_buy_to_sell)
{
    book b;
    std::vector<std::tuple<order_id, order_size, order_price>> cbs;
    auto cb = [&](order_id id, order_size s, order_price p){
        cbs.push_back({ id, s, p});
        return 0;
    };
    b.post_order_complete_callback(std::function(cb));

    auto sid = b.limit_sell(100, 100);
    auto bid = b.limit_buy(100, 100);


    EXPECT_TRUE(std::get<0>(cbs[0]) == sid || std::get<0>(cbs[0]) == bid);
    EXPECT_TRUE(std::get<0>(cbs[1]) == sid || std::get<0>(cbs[1]) == bid);
    EXPECT_NE(std::get<0>(cbs[0]), std::get<0>(cbs[1]));
}

TEST(book_tests, can_match_sell_to_buy)
{
    book b;
    std::vector<std::tuple<order_id, order_size, order_price>> cbs;
    auto cb = [&](order_id id, order_size s, order_price p){
        cbs.push_back({ id, s, p});
        return 0;
    };
    b.post_order_complete_callback(std::function(cb));

    auto sid = b.limit_buy(100, 100);
    auto bid = b.limit_sell(100, 100);


    EXPECT_TRUE(std::get<0>(cbs[0]) == sid || std::get<0>(cbs[0]) == bid);
    EXPECT_TRUE(std::get<0>(cbs[1]) == sid || std::get<0>(cbs[1]) == bid);
    EXPECT_NE(std::get<0>(cbs[0]), std::get<0>(cbs[1]));
}

