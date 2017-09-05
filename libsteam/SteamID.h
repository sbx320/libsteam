#pragma once
#include <cstdint>

namespace steam {
	namespace SteamID {
        inline uint32_t ToAccountID(uint64_t steamid)
        {
            return static_cast<uint32_t>(steamid - 76561197960265728ULL);
        }

        inline uint64_t ToSteamID(uint32_t accountid)
        {
            return static_cast<uint64_t>(accountid) + 76561197960265728ULL;
        }
	};
}