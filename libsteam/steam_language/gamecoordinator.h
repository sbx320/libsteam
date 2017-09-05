#pragma once 

#include <cstdint>
#include <limits>

namespace steam
{
    struct MsgGCHdrProtoBuf
    {
        uint32_t msg = 0;
        int32_t headerLength = 0;
    };

    struct MsgGCHdr
    {
        uint16_t headerVersion = 1;

        uint64_t targetJobID = std::numeric_limits<uint64_t>::max();
        uint64_t sourceJobID = std::numeric_limits<uint64_t>::max();
    };
}