#pragma once 

#include "header.h"
#include "gamecoordinator.h"
namespace steam
{

    struct MsgClientJustStrings
    {
    };

    struct MsgClientGenericResponse
    {
        EResult result = EResult::Invalid;
    };

    struct MsgChannelEncryptRequest
    {
        static constexpr uint32_t PROTOCOL_VERSION = 1;

        uint32_t protocolVersion = MsgChannelEncryptRequest::PROTOCOL_VERSION;
        EUniverse universe = EUniverse::Invalid;
    };

    struct MsgChannelEncryptResponse
    {
        uint32_t protocolVersion = MsgChannelEncryptRequest::PROTOCOL_VERSION;
        uint32_t keySize = 128;
    };

    struct MsgChannelEncryptResult
    {
        EResult result = EResult::Invalid;
    };

    struct MsgClientNewLoginKey
    {
        uint32_t uniqueID = 0;
        uint8_t loginKey[20];
    };

    struct MsgClientNewLoginKeyAccepted
    {
        uint32_t uniqueID = 0;
    };

    struct MsgClientLogon
    {
        const uint32_t ObfuscationMask = 0xBAADF00D;
        const uint32_t CurrentProtocol = 65579;

        const uint32_t ProtocolVerMajorMask = 0xFFFF0000;
        const uint32_t ProtocolVerMinorMask = 0xFFFF;

        const uint16_t ProtocolVerMinorMinGameServers = 4;
        const uint16_t ProtocolVerMinorMinForSupportingEMsgMulti = 12;
        const uint16_t ProtocolVerMinorMinForSupportingEMsgClientEncryptPct = 14;
        const uint16_t ProtocolVerMinorMinForExtendedMsgHdr = 17;
        const uint16_t ProtocolVerMinorMinForCellId = 18;
        const uint16_t ProtocolVerMinorMinForSessionIDLast = 19;
        const uint16_t ProtocolVerMinorMinForServerAvailablityMsgs = 24;
        const uint16_t ProtocolVerMinorMinClients = 25;
        const uint16_t ProtocolVerMinorMinForOSType = 26;
        const uint16_t ProtocolVerMinorMinForCegApplyPESig = 27;
        const uint16_t ProtocolVerMinorMinForMarketingMessages2 = 27;
        const uint16_t ProtocolVerMinorMinForAnyProtoBufMessages = 28;
        const uint16_t ProtocolVerMinorMinForProtoBufLoggedOffMessage = 28;
        const uint16_t ProtocolVerMinorMinForProtoBufMultiMessages = 28;
        const uint16_t ProtocolVerMinorMinForSendingProtocolToUFS = 30;
        const uint16_t ProtocolVerMinorMinForMachineAuth = 33;
        const uint16_t ProtocolVerMinorMinForSessionIDLastAnon = 36;
        const uint16_t ProtocolVerMinorMinForEnhancedAppList = 40;
        const uint16_t ProtocolVerMinorMinForGzipMultiMessages = 43;
    };

    struct MsgClientVACBanStatus
    {
        uint32_t numBans = 0;
    };

    struct MsgClientAppUsageEvent
    {
        EAppUsageEvent appUsageEvent;
        uint64_t gameID;
        uint16_t offline;
    };

    struct MsgClientEmailAddrInfo
    {
        uint32_t passwordStrength;
        uint32_t flagsAccountSecurityPolicy;
        uint8_t validated;
    };

    struct MsgClientUpdateGuestPassesList
    {
        EResult result;
        int countGuestPassesToGive;
        int countGuestPassesToRedeem;
    };

    struct MsgClientRequestedClientStats
    {
        int countStats;
    };


    struct MsgClientP2PIntroducerMessage
    {
        uint64_t steamID;
        EIntroducerRouting routingType;
        uint8_t data;
        uint32_t dataLen;
    };

    struct MsgClientOGSBeginSession
    {
        uint8_t accountType;
        uint64_t accountId;
        uint32_t appId;
        uint32_t timeStarted;
    };

    struct MsgClientOGSBeginSessionResponse
    {
        EResult result;
        uint8_t collectingAny;
        uint8_t collectingDetails;
        uint64_t sessionId;
    };

    struct MsgClientOGSEndSession
    {
        uint64_t sessionId;
        uint32_t timeEnded;
        int reasonCode;
        int countAttributes;
    };

    struct MsgClientOGSEndSessionResponse
    {
        EResult result;
    };


    struct MsgClientOGSWriteRow
    {
        uint64_t sessionId;
        int countAttributes;
    };

    struct MsgClientGetFriendsWhoPlayGame
    {
        uint64_t gameId;
    };

    struct MsgClientGetFriendsWhoPlayGameResponse
    {
        EResult result;
        uint64_t gameId;
        uint32_t countFriends;
    };

    struct MsgGSPerformHardwareSurvey
    {
        uint32_t flags;
    };

    struct MsgGSGetPlayStatsResponse
    {
        EResult result;
        int rank;
        uint32_t lifetimeConnects;
        uint32_t lifetimeMinutesPlayed;
    };

    struct MsgGSGetReputationResponse
    {
        EResult result;
        uint32_t reputationScore;
        uint8_t banned;
        uint32_t bannedIp;
        uint16_t bannedPort;
        uint64_t bannedGameId;
        uint32_t timeBanExpires;
    };

    struct MsgGSDeny
    {
        uint64_t steamId;
        EDenyReason denyReason;
    };

    struct MsgGSApprove
    {
        uint64_t steamId;
    };

    struct MsgGSKick
    {
        uint64_t steamId;
        EDenyReason denyReason;
        int waitTilMapChange;
    };

    struct MsgGSGetUserGroupStatus
    {
        uint64_t steamIdUser;
        uint64_t steamIdGroup;
    };

    struct MsgGSGetUserGroupStatusResponse
    {
        uint64_t steamIdUser;
        uint64_t steamIdGroup;
        EClanRelationship clanRelationship;
        EClanRank clanRank;
    };

    struct MsgClientJoinChat
    {
        uint64_t steamIdChat;
        uint8_t isVoiceSpeaker;
    };

    struct MsgClientChatEnter
    {
        uint64_t steamIdChat;
        uint64_t steamIdFriend;

        EChatRoomType chatRoomType;

        uint64_t steamIdOwner;
        uint64_t steamIdClan;

        uint8_t chatFlags;

        EChatRoomEnterResponse enterResponse;

        int numMembers;
    };

    struct MsgClientChatMsg
    {
        uint64_t steamIdChatter;
        uint64_t steamIdChatRoom;
        EChatEntryType chatMsgType;
    };

    struct MsgClientChatMemberInfo
    {
        uint64_t steamIdChat;
        EChatInfoType type;
    };

    struct MsgClientChatAction
    {
        uint64_t steamIdChat;
        uint64_t steamIdUserToActOn;
        EChatAction chatAction;
    };

    struct MsgClientChatActionResult
    {
        uint64_t steamIdChat;
        uint64_t steamIdUserActedOn;
        EChatAction chatAction;
        EChatActionResult actionResult;
    };

    struct MsgClientChatRoomInfo
    {
        uint64_t steamIdChat;
        EChatInfoType type;
    };

    struct MsgClientSetIgnoreFriend
    {
        uint64_t mySteamId;
        uint64_t steamIdFriend;

        uint8_t ignore;
    };

    struct MsgClientSetIgnoreFriendResponse
    {
        uint64_t unknown;

        EResult result;
    };

    struct MsgClientLoggedOff
    {
        EResult result;
        int secMinReconnectHint;
        int secMaxReconnectHint;
    };

    struct MsgClientLogOnResponse
    {
        EResult result;
        int outOfGameHeartbeatRateSec;
        int inGameHeartbeatRateSec;
        uint64_t clientSuppliedSteamId;
        uint32_t ipPublic;
        uint32_t serverRealTime;
    };

    struct MsgClientSendGuestPass
    {
        uint64_t giftId;
        uint8_t giftType;
        uint32_t accountId;
    };

    struct MsgClientSendGuestPassResponse
    {
        EResult result;
    };

    struct MsgClientServerUnavailable
    {
        uint64_t jobidSent;
        uint32_t eMsgSent;
        EServerType eServerTypeUnavailable;
    };

    struct MsgClientCreateChat
    {
        EChatRoomType chatRoomType;

        uint64_t gameId;

        uint64_t steamIdClan;

        EChatPermission permissionOfficer;
        EChatPermission permissionMember;
        EChatPermission permissionAll;

        uint32_t membersMax;

        uint8_t chatFlags;

        uint64_t steamIdFriendChat;
        uint64_t steamIdInvited;
    };

    struct MsgClientCreateChatResponse
    {
        EResult result;
        uint64_t steamIdChat;
        EChatRoomType chatRoomType;
        uint64_t steamIdFriendChat;
    };

    struct MsgClientMarketingMessageUpdate2
    {
        uint32_t marketingMessageUpdateTime;
        uint32_t count;
    };
}