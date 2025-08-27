#include <iostream>

#include "client.hpp"
#include "character_buffer.hpp"

int main(int argc, const char *argv[])
{
    UDSClient<StringBufferWithMetaData> client{};

    auto data = StringBufferWithMetaData{"NerdMan", 
                                        "This guy transfers objects by magic!", 
                                        21};

    if (auto ret = client.send_msg(data)) {}
    else
    {
        std::cout << "No info gathered for send failure.\n";
    }

    return 0;
}
