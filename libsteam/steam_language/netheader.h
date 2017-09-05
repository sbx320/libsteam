#pragma once
#include <cstdint>

namespace steam
{

    enum class EUdpPacketType : uint8_t
    {
        Invalid = 0,

        ChallengeReq = 1,
        Challenge = 2,
        Connect = 3,
        Accept = 4,
        Disconnect = 5,
        Data = 6,
        Datagram = 7,
        Max = 8,
    };

    struct UdpHeader
    {
        const uint32_t MAGIC = 0x31305356;

        uint32_t magic = UdpHeader::MAGIC;

        uint16_t payloadSize;
        EUdpPacketType packetType = EUdpPacketType::Invalid;
        uint8_t flags;

        uint32_t sourceConnID = 512;
        uint32_t destConnID;

        uint32_t seqThis;
        uint32_t seqAck;

        uint32_t packetsInMsg;
        uint32_t msgStartSeq;

        uint32_t msgSize;
    };

    struct ChallengeData
    {
        static constexpr uint32_t CHALLENGE_MASK = 0xA426DF2B;

        uint32_t challengeValue;
        uint32_t serverLoad;
    };

    struct ConnectData
    {
        const uint32_t CHALLENGE_MASK = ChallengeData::CHALLENGE_MASK;

        uint32_t challengeValue;
    };

    struct Accept
    {
    };

    struct Datagram
    {
    };

    struct Disconnect
    {
    };
}