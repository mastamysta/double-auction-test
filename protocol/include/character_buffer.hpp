#pragma once

struct StringBufferWithMetaData
{
    static constexpr int BUFFER_LENGTH = 255;
    static constexpr int BITS_IN_BYTE = 8;
    char protocol_id[BITS_IN_BYTE];
    char buffer[BUFFER_LENGTH];
    size_t client_id;
};

