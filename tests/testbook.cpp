#include <iostream>
#include <tuple>

#include "gtest/gtest.h"

#include "book.hpp"

class BasicOrderBookTest: public testing::Test
{
protected:
    Book b;

    auto setUp()
    {
        std::cout << "Doing setup\n";
        b = {};
    }
};

TEST_F(BasicOrderBookTest, can_buy)
{
    b.limit_buy(100, 100);
}

TEST_F(BasicOrderBookTest, can_sell)
{
    b.limit_sell(100, 100);
}

TEST_F(BasicOrderBookTest, buy_ids_are_unique)
{
    ASSERT_NE(b.limit_buy(100, 100), b.limit_buy(100, 100));
}

TEST_F(BasicOrderBookTest, sell_ids_are_unique)
{
    ASSERT_NE(b.limit_sell(100, 100), b.limit_sell(100, 100));
}


TEST_F(BasicOrderBookTest, can_match_buy_to_sell)
{
    std::vector<std::tuple<order_id, order_size, order_price>> cbs;
    auto cb = [&](order_id id, order_size s, order_price p){
        cbs.push_back({ id, s, p});
        return 0;
    };
    b.post_order_complete_callback(std::function(cb));

    auto sid = b.limit_sell(100, 100);
    auto bid = b.limit_buy(100, 100);

    EXPECT_EQ(std::get<0>(cbs[0]), sid);
    EXPECT_EQ(bid, -1);
}

TEST_F(BasicOrderBookTest, can_match_sell_to_buy)
{
    std::vector<std::tuple<order_id, order_size, order_price>> cbs;
    auto cb = [&](order_id id, order_size s, order_price p){
        cbs.push_back({ id, s, p});
        return 0;
    };
    b.post_order_complete_callback(std::function(cb));

    auto bid = b.limit_buy(100, 100);
    auto sid = b.limit_sell(100, 100);

    EXPECT_EQ(std::get<0>(cbs[0]), bid);
    EXPECT_EQ(sid, -1);
}

TEST_F(BasicOrderBookTest, matches_best_existing_buy_to_sell)
{
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

TEST_F(BasicOrderBookTest, matches_best_existing_sell_to_buy)
{
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

TEST_F(BasicOrderBookTest, matches_sell_to_earliest_valid_buy)
{
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

TEST_F(BasicOrderBookTest, matches_buy_to_earliest_valid_sell)
{
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

TEST_F(BasicOrderBookTest, can_cancel_buy)
{
    auto bid = b.limit_buy(100, 100);
    ASSERT_TRUE(b.cancel_order(bid));
}

TEST_F(BasicOrderBookTest, can_cancel_sell)
{
    auto sid = b.limit_sell(100, 100);
    ASSERT_TRUE(b.cancel_order(sid));
}

TEST_F(BasicOrderBookTest, cannot_cancel_bad_id)
{
    auto sid = b.limit_sell(100, 100);
    ASSERT_FALSE(b.cancel_order(101));
}

TEST_F(BasicOrderBookTest, can_fok_sell)
{
    std::vector<std::tuple<order_id, order_size, order_price>> cbs;
    auto cb = [&](order_id id, order_size s, order_price p){
        cbs.push_back({ id, s, p});
        return 0;
    };
    b.post_order_complete_callback(std::function(cb));

    auto bid = b.limit_buy(100, 100);
    auto fok_success = b.fok_sell(50, 25);

    EXPECT_EQ(std::get<0>(cbs[0]), bid);
    EXPECT_EQ(std::get<1>(cbs[0]), 50); // Size
    EXPECT_EQ(std::get<2>(cbs[0]), 100); // Price

    ASSERT_TRUE(fok_success);
}

TEST_F(BasicOrderBookTest, doesnt_fill_with_worse_price)
{
    std::vector<std::tuple<order_id, order_size, order_price>> cbs;
    auto cb = [&](order_id id, order_size s, order_price p){
        cbs.push_back({ id, s, p});
        return 0;
    };
    b.post_order_complete_callback(std::function(cb));

    auto bid = b.limit_buy(100, 100);
    auto fok_success = b.fok_sell(50, 125);

    ASSERT_TRUE(cbs.empty());
    ASSERT_FALSE(fok_success);
}
