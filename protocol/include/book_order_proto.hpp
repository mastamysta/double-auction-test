#pragma once

namespace order_protocol
{

using PriceType = size_t;
using VolumeType = size_t;
using OrderIDType = size_t;

enum class MessageTypeID
{
    LIMIT,
    FOK,
    CANCEL
};

enum class Side
{
    BUY,
    SELL
};

struct LimitDetails
{
    PriceType price;
    VolumeType volume;
    Side side;
};

struct FOKDetails : LimitDetails {};

struct CancelDetails
{
    OrderIDType order_id;
};

struct GenericMessage
{
    MessageTypeID message_type;

    union { LimitDetails lim;
            FOKDetails fok;
            CancelDetails can; } details;
};

}
