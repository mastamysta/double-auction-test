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

TEST(book_tests, buy_ids_are_unique)
{
    book b;
    ASSERT_NE(b.limit_buy(100, 100), b.limit_buy(100, 100));
}

TEST(book_tests, sell_ids_are_unique)
{
    book b;
    ASSERT_NE(b.limit_sell(100, 100), b.limit_sell(100, 100));
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

    EXPECT_TRUE(std::get<0>(cbs[0]) == sid);
    EXPECT_EQ(bid, -1);
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

    auto bid = b.limit_buy(100, 100);
    auto sid = b.limit_sell(100, 100);

    EXPECT_TRUE(std::get<0>(cbs[0]) == bid);
    EXPECT_EQ(sid, -1);
}

TEST(book_tests, matches_best_existing_buy_to_sell)
{
    book b;
    std::vector<std::tuple<order_id, order_size, order_price>> cbs;
    auto cb = [&](order_id id, order_size s, order_price p){
        cbs.push_back({ id, s, p});
        return 0;
    };
    b.post_order_complete_callback(std::function(cb));

    b.limit_buy(100, 80);
    auto bid = b.limit_buy(100, 200);
    b.limit_buy(100, 130);
    b.limit_buy(100, 150);

    auto sid = b.limit_sell(100, 100);

    EXPECT_EQ(std::get<0>(cbs[0]), bid);
    EXPECT_EQ(std::get<1>(cbs[0]), 100);
    EXPECT_EQ(std::get<2>(cbs[0]), 200);
    EXPECT_EQ(sid, -1);
}

TEST(book_tests, matches_best_existing_sell_to_buy)
{
    book b;
    std::vector<std::tuple<order_id, order_size, order_price>> cbs;
    auto cb = [&](order_id id, order_size s, order_price p){
        cbs.push_back({ id, s, p});
        return 0;
    };
    b.post_order_complete_callback(std::function(cb));

    b.limit_sell(100, 250);
    auto sid = b.limit_sell(100, 100);
    b.limit_sell(100, 1700);
    b.limit_sell(100, 140);

    auto bid = b.limit_buy(100, 200);

    EXPECT_EQ(std::get<0>(cbs[0]), sid);
    EXPECT_EQ(std::get<1>(cbs[0]), 100);
    EXPECT_EQ(std::get<2>(cbs[0]), 100);
    EXPECT_EQ(bid, -1);
}

TEST(book_tests, matches_sell_to_earliest_valid_buy)
{
    book b;
    std::vector<std::tuple<order_id, order_size, order_price>> cbs;
    auto cb = [&](order_id id, order_size s, order_price p){
        cbs.push_back({ id, s, p});
        return 0;
    };
    b.post_order_complete_callback(std::function(cb));

    b.limit_buy(100, 70);
    auto bid = b.limit_buy(100, 100);
    b.limit_buy(100, 100);
    b.limit_buy(100, 80);

    auto sid = b.limit_sell(100, 100);

    EXPECT_EQ(std::get<0>(cbs[0]), bid);
    EXPECT_EQ(std::get<1>(cbs[0]), 100);
    EXPECT_EQ(std::get<2>(cbs[0]), 100);
    EXPECT_EQ(sid, -1);
}

TEST(book_tests, matches_buy_to_earliest_valid_sell)
{
    book b;
    std::vector<std::tuple<order_id, order_size, order_price>> cbs;
    auto cb = [&](order_id id, order_size s, order_price p){
        cbs.push_back({ id, s, p});
        return 0;
    };
    b.post_order_complete_callback(std::function(cb));

    b.limit_sell(100, 250);
    auto sid = b.limit_sell(100, 100);
    b.limit_sell(100, 100);
    b.limit_sell(100, 140);

    auto bid = b.limit_buy(100, 200);

    EXPECT_EQ(std::get<0>(cbs[0]), sid);
    EXPECT_EQ(std::get<1>(cbs[0]), 100);
    EXPECT_EQ(std::get<2>(cbs[0]), 100);
    EXPECT_EQ(bid, -1);
}
