#pragma once

#include "emsg.h"
#include "eresult.h"
#include "enums.h"
#include "netheader.h"
#include <limits>
namespace steam
{

#pragma pack(push, 1)
    struct MsgHdr
    {
        EMsg msg = EMsg::Invalid;

        uint64_t targetJobID = std::numeric_limits<uint64_t>::max();
        uint64_t sourceJobID = std::numeric_limits<uint64_t>::max();
    };
	
    struct ExtendedClientMsgHdr
    {
        EMsg msg = EMsg::Invalid;

        uint8_t headerSize = 36;

        uint16_t headerVersion = 2;

        uint64_t targetJobID = std::numeric_limits<uint64_t>::max();
        uint64_t sourceJobID = std::numeric_limits<uint64_t>::max();

        uint8_t headerCanary = 239;

        uint64_t steamID = 0;
        int32_t sessionID = 0;
    };

    struct MsgHdrProtoBuf
    {
        EMsg msg = EMsg::Invalid;
        int32_t headerLength = 0;
    };
#pragma pack(pop)


}