#include "Dota.h"
#include <dota/dota_gcmessages_common.pb.h>
#include <dota/dota_gcmessages_client.pb.h>
#include <dota/dota_gcmessages_client_chat.pb.h>
#include <dota/dota_gcmessages_client_match_management.pb.h>
#include <steam/steammessages_base.pb.h>
#include <steam/steammessages_clientserver.pb.h>
#include <steam/steammessages_clientserver_2.pb.h>
#include <dota/gcsystemmsgs.pb.h>
#include <dota/dota_gcmessages_msgid.pb.h>
#include <boost/asio/steady_timer.hpp>

steam::Dota::Dota(boost::asio::io_service& io)
    : GC(io, 570), _gcTimer(io) 
{
    _gcTimer.SetCallback([this](auto) { this->Hello(); });
    GCMessage.connect([this](auto a, auto b, auto c, auto d)
    {
        this->HandleMessage(a, b, c, d);
    });
    Disconnected.connect([this](auto)
    {
        this->_gcTimer.Stop();
        this->_gcConnected = false;
        this->Lobby.reset();
    });
}

steam::Dota::~Dota() {}

void steam::Dota::Start()
{
    std::vector<uint8_t> appUsageMessage(sizeof(MsgClientAppUsageEvent));
    auto appUsage = new (appUsageMessage.data()) MsgClientAppUsageEvent;
    appUsage->appUsageEvent = EAppUsageEvent::GameLaunch;
    appUsage->gameID = _appId;

    WriteMessage(EMsg::ClientAppUsageEvent, appUsageMessage);

	proto::steam::CMsgClientGamesPlayed msg;
	proto::steam::CMsgClientGamesPlayed_GamePlayed *gamePlayed = msg.add_games_played();
	gamePlayed->set_game_id(_appId);
	gamePlayed->set_game_extra_info("Dota 2");
	gamePlayed->set_streaming_provider_id(0);
	gamePlayed->set_game_flags(1); // Source 2
	gamePlayed->set_owner_id(SteamID::ToAccountID(steamID));

	msg.set_client_os_type(16);
	WriteMessage(EMsg::ClientGamesPlayedWithDataBlob, msg);

    _gcConnected = false;

	Hello();
}


void steam::Dota::Hello()
{
    if (_gcConnected)
    {
        _gcTimer.Stop();
        return;
    }

	using namespace proto::dota;
	CMsgClientHello clientHello;
	clientHello.set_client_launcher(PartnerAccountType::PARTNER_NONE);
	clientHello.set_engine(ESourceEngine::k_ESE_Source2);
	clientHello.set_secret_key("");
	clientHello.set_client_session_need(104);
	SendGC(EGCBaseClientMsg::k_EMsgGCClientHello, clientHello);

	using namespace std::chrono;
    _gcTimer.Start(5s);
}

void steam::Dota::Stop()
{
	using namespace proto::steam;
    WriteMessage(EMsg::ClientGamesPlayed, CMsgClientGamesPlayed{});
    _gcTimer.Stop();
    Lobby.reset();
}

void steam::Dota::CreateLobby(const std::string & name, const std::string & password, uint32_t leagueid, uint32_t server)
{
	using namespace proto::dota;
	CMsgPracticeLobbyCreate msg; 
	CMsgPracticeLobbySetDetails& details = *msg.mutable_lobby_details();
	msg.set_pass_key(password);
	msg.set_search_key("");
	details.set_game_name(name);
	details.set_pass_key(password);
    if(leagueid)
        details.set_leagueid(leagueid);
    details.set_game_mode(2); // -cm 
    details.set_server_region(server);
	details.set_visibility(DOTALobbyVisibility::DOTALobbyVisibility_Public);
    details.set_allow_spectating(true);

	SendGC(EDOTAGCMsg::k_EMsgGCPracticeLobbyCreate, msg);
}

void steam::Dota::LeaveLobby()
{
    using namespace proto::dota;
    CMsgPracticeLobbyLeave msg;
    SendGC(k_EMsgGCPracticeLobbyLeave, msg);

    CMsgAbandonCurrentGame msg2;
    SendGC(k_EMsgGCAbandonCurrentGame, msg2);
}

void steam::Dota::InvitePlayer(uint32_t account_id)
{
	using namespace proto::dota;
	CMsgInviteToLobby invite;
	invite.set_steam_id(SteamID::ToSteamID(account_id));
	SendGC(EGCBaseMsg::k_EMsgGCInviteToLobby, invite);
	if (Lobby)
	{
		proto::steam::CMsgClientUDSInviteToGame msg;
		msg.set_steam_id_dest(SteamID::ToSteamID(account_id));
		std::string connectString = "+invite ";
		connectString += std::to_string(Lobby->lobby_id());
		connectString += " -launchsource2";
		msg.set_connect_string(connectString);

		WriteMessage(EMsg::ClientUDSInviteToGame, msg);
	}
}

void steam::Dota::JoinChatChannel(const std::string & name, proto::dota::DOTAChatChannelType_t type)
{
	using namespace proto::dota;
	CMsgDOTAJoinChatChannel msg;
	msg.set_channel_name(name);
	msg.set_channel_type(type);

	SendGC(EDOTAGCMsg::k_EMsgGCJoinChatChannel, msg);
}

void steam::Dota::KickPlayerFromLobby(uint32_t account_id)
{
    using namespace proto::dota;
    CMsgPracticeLobbyKick msg;
    msg.set_account_id(account_id);
    SendGC(k_EMsgGCPracticeLobbyKick, msg);
}

void steam::Dota::KickPlayerFromLobbyTeam(uint32_t account_id)
{
    using namespace proto::dota;
    CMsgPracticeLobbyKickFromTeam msg;
    msg.set_account_id(account_id);
    SendGC(k_EMsgGCPracticeLobbyKickFromTeam, msg);
}

void steam::Dota::LaunchLobby()
{
    using namespace proto::dota;
    SendGC(k_EMsgGCPracticeLobbyLaunch, CMsgPracticeLobbyLaunch{});
}

void steam::Dota::SendChatMessage(uint64_t chatid, const std::string& message)
{
    using namespace proto::dota;
    CMsgDOTAChatMessage msg;
    msg.set_channel_id(chatid);
    msg.set_text(message);

    SendGC(k_EMsgGCChatMessage, msg);
}

void steam::Dota::FlipLobbyTeams()
{
    using namespace proto::dota;
    SendGC(k_EMsgGCFlipLobbyTeams, CMsgFlipLobbyTeams{});
}


void steam::Dota::HandleMessage(uint32_t emsg, const uint8_t * data, std::size_t length, uint64_t jobid)
{
	switch (emsg)
	{
		using namespace proto::dota;
	case EGCBaseClientMsg::k_EMsgGCClientWelcome:
		return HandleWelcome(data, length);
	case EGCBaseClientMsg::k_EMsgGCPingRequest:
		return HandlePingRequest(data, length);
	case ESOMsg::k_ESOMsg_UpdateMultiple:
		return HandleUpdateMultiple(data, length);
	case ESOMsg::k_ESOMsg_CacheSubscribed:
		return HandleCacheSubscribed(data, length);
	case ESOMsg::k_ESOMsg_CacheUnsubscribed:
		return HandleCacheUnsubscribed(data, length);
	case ESOMsg::k_ESOMsg_Destroy:
		return HandleCacheDestroy(data, length);
	case k_EMsgGCClientConnectionStatus:
		return HandleConnectionStatus(data, length);
	case k_EMsgGCJoinChatChannelResponse:
		return HandleJoinChatChannelResponse(data, length);
	case k_EMsgGCChatMessage:
		return HandleChatMessage(data, length);
    case k_EMsgClientToGCGetProfileCardResponse:
        return HandleProfileCards(data, length);
	}

}

proto::dota::CMsgPracticeLobbySetDetails steam::Dota::GetLobbyDetails()
{
    using namespace proto::dota;
    CMsgPracticeLobbySetDetails details;
    details.set_allchat(Lobby->allchat());
    details.set_allow_cheats(Lobby->allow_cheats());
    details.set_allow_spectating(Lobby->allow_spectating());
    details.set_bot_difficulty_radiant(Lobby->bot_difficulty_radiant());
    details.set_bot_difficulty_dire(Lobby->bot_difficulty_dire());
    details.set_cm_pick(Lobby->cm_pick());
    details.set_custom_difficulty(Lobby->custom_difficulty());
    details.set_custom_game_id(Lobby->custom_game_id());
    details.set_custom_game_mode(Lobby->custom_game_mode());
    details.set_custom_map_name(Lobby->custom_map_name());
    details.set_dire_series_wins(Lobby->dire_series_wins());
    details.set_dota_tv_delay(Lobby->dota_tv_delay());
    details.set_fill_with_bots(Lobby->fill_with_bots());
    details.set_game_mode(Lobby->game_mode());
    details.set_game_name(Lobby->game_name());
    details.set_game_version(Lobby->game_version());
    details.set_intro_mode(Lobby->intro_mode());
    details.set_lan(Lobby->lan());
    details.set_leagueid(Lobby->leagueid());
    details.set_load_game_id(Lobby->load_game_id());
    details.set_lobby_id(Lobby->lobby_id());
    details.set_pass_key(Lobby->pass_key());
    details.set_penalty_level_dire(Lobby->penalty_level_dire());
    details.set_penalty_level_radiant(Lobby->penalty_level_radiant());
    details.set_radiant_series_wins(Lobby->radiant_series_wins());
    details.set_series_type(Lobby->series_type());
    details.set_server_region(Lobby->server_region());
    return details;
}

void steam::Dota::HandleWelcome(const uint8_t * data, std::size_t length)
{
	using namespace proto::dota;
	CMsgClientWelcome msg;
	msg.ParseFromArray(data, length);

    // This has to be above the lobby parsing to ensure that we'll go from 
    // the gc connected callback to the in lobby callback rather than the 
    // other way around
    _gcConnected = true;
    _gcTimer.Stop();
    GCConnected();

	for (auto&& cache : msg.outofdate_subscribed_caches())
	{
		for (auto&& obj : cache.objects())
		{
			if (obj.type_id() == 2004) // Lobby
			{
				UpdateLobby(obj.object_data(0));
			}
		}
	}

}

void steam::Dota::HandlePingRequest(const uint8_t * data, std::size_t length)
{
	using namespace proto::dota;
	// We don't actually need to parse the message
	SendGC(EGCBaseClientMsg::k_EMsgGCPingResponse, CMsgGCClientPing{});
}

void steam::Dota::HandleUpdateMultiple(const uint8_t * data, std::size_t length)
{
	using namespace proto::dota;
	CMsgSOMultipleObjects msg;
	msg.ParseFromArray(data, length);

	for (auto&& obj : msg.objects_modified())
	{
		if (obj.type_id() == 2004) // Lobby
		{
			UpdateLobby(obj.object_data());
		}
	}
}

void steam::Dota::HandleCacheSubscribed(const uint8_t * data, std::size_t length)
{
	using namespace proto::dota;
	CMsgSOCacheSubscribed msg;
	msg.ParseFromArray(data, length);
	for(auto&& obj : msg.objects())
	{
		if (obj.type_id() == 2004) // Lobby
		{
			UpdateLobby(obj.object_data(0));
		}
	}
}

void steam::Dota::HandleCacheUnsubscribed(const uint8_t * data, std::size_t length)
{
	using namespace proto::dota;
	CMsgSOCacheUnsubscribed msg;
	msg.ParseFromArray(data, length);

	if(Lobby && msg.owner_soid().id() == Lobby->lobby_id())
	{
		Lobby.reset();
		LobbyChanged(nullptr);
	}
}

void steam::Dota::HandleCacheDestroy(const uint8_t * data, std::size_t length)
{
	using namespace proto::dota;
	CMsgSOSingleObject msg;
	msg.ParseFromArray(data, length);

	if (Lobby && msg.type_id() == 2004) // Lobby
	{
		Lobby.reset();
		LobbyChanged(nullptr);
	}
}

void steam::Dota::UpdateLobby(const std::string& data)
{
	using namespace proto::dota;
    Lobby = std::make_unique<CSODOTALobby>();
	Lobby->ParseFromString(data);
	LobbyChanged(Lobby.get());
}

void steam::Dota::HandleConnectionStatus(const uint8_t * data, std::size_t length)
{
	using namespace proto::dota;
	CMsgConnectionStatus msg;
	msg.ParseFromArray(data, length);

    if (msg.status() != GCConnectionStatus_HAVE_SESSION)
    {
        _gcConnected = false;
        using namespace std::chrono;
        _gcTimer.Start(5s);
        GCDisconnected();
    }
}

void steam::Dota::HandleJoinChatChannelResponse(const uint8_t * data, std::size_t length)
{
	using namespace proto::dota;
	CMsgDOTAJoinChatChannelResponse msg;
	msg.ParseFromArray(data, length);
    ChatJoined(msg.channel_id(), msg.channel_type());
}

void steam::Dota::HandleChatMessage(const uint8_t * data, std::size_t length)
{
	using namespace proto::dota;
	CMsgDOTAChatMessage msg;
	msg.ParseFromArray(data, length);
    ChatMessage(msg.account_id(), msg.persona_name(), msg.text());
}

void steam::Dota::SetLobbyFirstpick(proto::dota::DOTA_CM_PICK fp)
{
    using namespace proto::dota;
    CMsgPracticeLobbySetDetails msg = GetLobbyDetails();
    msg.set_cm_pick(fp);
    SendGC(k_EMsgGCPracticeLobbySetDetails, msg);
}

void steam::Dota::SetLobbyServer(uint32_t server)
{
    using namespace proto::dota;
    CMsgPracticeLobbySetDetails msg = GetLobbyDetails();
    msg.set_server_region(server);
    SendGC(k_EMsgGCPracticeLobbySetDetails, msg);
}

void steam::Dota::SetLobbyGamemode(uint32_t gamemode)
{
    using namespace proto::dota;
    CMsgPracticeLobbySetDetails msg = GetLobbyDetails();
    msg.set_game_mode(gamemode);
    SendGC(k_EMsgGCPracticeLobbySetDetails, msg);
}

void steam::Dota::RequestProfileCards(uint32_t account_id)
{
    using namespace proto::dota;
    CMsgDOTAProfileRequest msg;
    msg.set_account_id(account_id);
    SendGC(k_EMsgClientToGCGetProfileCard, msg);
}

void steam::Dota::HandleProfileCards(const uint8_t * data, std::size_t length)
{
    using namespace proto::dota;
    CMsgDOTAProfileCard msg;
    msg.ParseFromArray(data, length);
    ProfileCardsReceived(msg);
}