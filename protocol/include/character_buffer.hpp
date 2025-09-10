#pragma once

struct StringBufferWithMetaData
{
    static constexpr int BUFFER_LENGTH = 255;
    static constexpr int PROTOCOL_ID_LENGTH = 8;
    char protocol_id[PROTOCOL_ID_LENGTH];
    char buffer[BUFFER_LENGTH];
    size_t client_id;
};

