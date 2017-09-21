#pragma once

#include "steam_language/steam_language.h"
#include "RecurringTimer.h"

#include "SteamID.h"

#include <cryptopp/osrng.h>
#include <boost/asio.hpp>
#include <ksignals.h>
#include <steam/steammessages_base.pb.h>
#include <steam/steammessages_clientserver.pb.h>
#include <steam/steammessages_clientserver_2.pb.h>
#include <steam/steammessages_clientserver_login.pb.h>

namespace steam 
{
class CMClient 
{
public:
	CMClient(boost::asio::io_service& io);

	void Connect(boost::asio::ip::tcp::resolver::iterator & endpoint);
    uint64_t steamID = 76561197960265728ULL;

	void Disconnect();
	bool IsConnected();
    ksignals::Event<void()> Connected;
    ksignals::Event<void(const boost::system::error_code&)> Disconnected;
    ksignals::Event<void(const proto::steam::CMsgClientLogonResponse&)> LoggedOn;
	ksignals::Event<void(const proto::steam::CMsgClientLoggedOff&)> LoggedOff;

    ksignals::Event<void(EMsg, const uint8_t*, std::size_t, uint64_t)> IncomingPacket;

protected:
    void WriteMessage(EMsg emsg, const std::vector<uint8_t>& data);
    void WriteMessage(steam::EMsg emsg, const google::protobuf::Message& message, std::uint64_t job_id = 0, uint32_t appid = 0);
    void WritePacket(const std::vector<uint8_t>& data);
    void ReadPacketHeader();
    void ReadPacket(uint32_t remainingSize, uint32_t expectedSize);

    int32_t _sessionID = 0;
    boost::asio::io_service& _io;

private:
	void Disconnect(const boost::system::error_code& ec);

	void DecryptAndHandle(std::size_t length);
	void HandleMessage(EMsg eMsg, const uint8_t* data, std::size_t length, std::uint64_t job_id);
	void SendHeartbeat(const boost::system::error_code& ec);

	void HandleEncryptRequest(const uint8_t* data, std::size_t length);
	void HandleEncryptResponse(const uint8_t* data, std::size_t length);
	void HandleMulti(const uint8_t* data, std::size_t length);
	void HandleLogOnResponse(const uint8_t* data, std::size_t length);
    void HandleLoggedOff(const uint8_t* data, std::size_t length);
	
    void HandlePacket(const uint8_t* data, std::size_t length);
    bool Inflate(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);

private:
	bool _handshakeComplete = false;
	uint8_t _sessionKey[32];
	CryptoPP::AutoSeededRandomPool _rng;
    std::vector<uint8_t> _readBuffer;
    uint32_t _readOffset;
    
	std::unique_ptr<boost::asio::ip::tcp::socket> _socket;
	RecurringTimer _heartbeat;

protected:
    static const uint32_t PROTO_MASK = 0x80000000;
};
}