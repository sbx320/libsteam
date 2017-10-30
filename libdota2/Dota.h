#pragma once
#include "GC.h"
#include <dota/dota_shared_enums.pb.h>

namespace steam
{
	class Dota : public GC
	{
	public:
        Dota(net::io_context& io);
        ~Dota();

		void Start();
		void Stop();
		void Hello();

		void CreateLobby(const std::string& name, const std::string& password, uint32_t leagueid, uint32_t server);
		void LeaveLobby();
		void InvitePlayer(uint32_t account_id);
		void JoinChatChannel(const std::string& name, proto::dota::DOTAChatChannelType_t type);
		void KickPlayerFromLobby(uint32_t account_id);
		void KickPlayerFromLobbyTeam(uint32_t account_id);
		void LaunchLobby();
        void SendChatMessage(uint64_t chatid, const std::string& message);
        void FlipLobbyTeams();
        void SetLobbyFirstpick(proto::dota::DOTA_CM_PICK fp);
        void SetLobbyServer(uint32_t server);
        void SetLobbyGamemode(uint32_t gamemode);
        void RequestProfileCards(uint32_t account_id);

		std::unique_ptr<proto::dota::CSODOTALobby> Lobby;

        ksignals::Event<void()> GCConnected;
        ksignals::Event<void()> GCDisconnected;
        ksignals::Event<void(uint64_t, proto::dota::DOTAChatChannelType_t)> ChatJoined;
        ksignals::Event<void(const proto::dota::CSODOTALobby*)> LobbyChanged;
        ksignals::Event<void(uint32_t, const std::string&, const std::string&)> ChatMessage;
        ksignals::Event<void(class proto::dota::CMsgDOTAProfileCard)> ProfileCardsReceived;

	private:
        proto::dota::CMsgPracticeLobbySetDetails GetLobbyDetails();

		void HandleWelcome(const uint8_t * data, std::size_t length);
		void HandlePingRequest(const uint8_t * data, std::size_t length);
		void HandleUpdateMultiple(const uint8_t * data, std::size_t length);
		void HandleCacheSubscribed(const uint8_t * data, std::size_t length);
		void HandleCacheUnsubscribed(const uint8_t * data, std::size_t length);
		void HandleCacheDestroy(const uint8_t * data, std::size_t length);
		void HandleConnectionStatus(const uint8_t * data, std::size_t length);
		void HandleJoinChatChannelResponse(const uint8_t * data, std::size_t length);
		void HandleChatMessage(const uint8_t * data, std::size_t length);
        void HandleProfileCards(const uint8_t * data, std::size_t length);

		void UpdateLobby(const std::string& data);

		void HandleMessage(uint32_t emsg, const uint8_t* data, std::size_t length, uint64_t jobid);

		bool _gcConnected = false;
		RecurringTimer _gcTimer;
	};
}
