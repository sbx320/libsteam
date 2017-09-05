#include "Client.h"
#include <steam/encrypted_app_ticket.pb.h>
#include <steam/steammessages_base.pb.h>
#include <steam/steammessages_clientserver_login.pb.h>
#include <steam/steammessages_clientserver_friends.pb.h>

steam::Client::Client(boost::asio::io_service& io)
: CMClient(io) 
{
}

void steam::Client::LogOn(const std::string& username, const std::string& password) {
	proto::steam::CMsgClientLogon logon;
	logon.set_account_name(username);
	logon.set_password(password);
	logon.set_protocol_version(65579);
	logon.set_should_remember_password(false);

	logon.set_client_os_type(-203); // Unknown Linux
	logon.set_client_language("english");
	logon.set_cell_id(0);
	logon.set_steam2_ticket_request(false);

	logon.set_client_package_version(1771);
	logon.set_supports_rate_limit_response(true);

    logon.set_eresult_sentryfile(static_cast<int>(EResult::FileNotFound));
	
	WriteMessage(EMsg::ClientLogon, logon);
}

void steam::Client::SetPersonaState(EPersonaState state) {
	proto::steam::CMsgClientChangeStatus change_status;
	change_status.set_persona_state(static_cast<google::protobuf::uint32>(state));
	WriteMessage(EMsg::ClientChangeStatus, change_status);
}
