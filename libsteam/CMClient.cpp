#include "CMClient.h"
#include "steam_language/steam_language.h"
#include <steam/encrypted_app_ticket.pb.h>
#include <steam/steammessages_base.pb.h>
#include <steam/steammessages_clientserver.pb.h>
#include <steam/steammessages_clientserver_login.pb.h>
#include <steam/steammessages_clientserver_2.pb.h>

#include <cryptopp/modes.h>
#include <cryptopp/rsa.h>
#include <cryptopp/crc.h>
#include <zlib/zlib.h>

static uint8_t steamPublicKey[] = {
	0x30, 0x81, 0x9D, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01,
	0x05, 0x00, 0x03, 0x81, 0x8B, 0x00, 0x30, 0x81, 0x87, 0x02, 0x81, 0x81, 0x00, 0xDF, 0xEC, 0x1A,
	0xD6, 0x2C, 0x10, 0x66, 0x2C, 0x17, 0x35, 0x3A, 0x14, 0xB0, 0x7C, 0x59, 0x11, 0x7F, 0x9D, 0xD3,
	0xD8, 0x2B, 0x7A, 0xE3, 0xE0, 0x15, 0xCD, 0x19, 0x1E, 0x46, 0xE8, 0x7B, 0x87, 0x74, 0xA2, 0x18,
	0x46, 0x31, 0xA9, 0x03, 0x14, 0x79, 0x82, 0x8E, 0xE9, 0x45, 0xA2, 0x49, 0x12, 0xA9, 0x23, 0x68,
	0x73, 0x89, 0xCF, 0x69, 0xA1, 0xB1, 0x61, 0x46, 0xBD, 0xC1, 0xBE, 0xBF, 0xD6, 0x01, 0x1B, 0xD8,
	0x81, 0xD4, 0xDC, 0x90, 0xFB, 0xFE, 0x4F, 0x52, 0x73, 0x66, 0xCB, 0x95, 0x70, 0xD7, 0xC5, 0x8E,
	0xBA, 0x1C, 0x7A, 0x33, 0x75, 0xA1, 0x62, 0x34, 0x46, 0xBB, 0x60, 0xB7, 0x80, 0x68, 0xFA, 0x13,
	0xA7, 0x7A, 0x8A, 0x37, 0x4B, 0x9E, 0xC6, 0xF4, 0x5D, 0x5F, 0x3A, 0x99, 0xF9, 0x9E, 0xC4, 0x3A,
	0xE9, 0x63, 0xA2, 0xBB, 0x88, 0x19, 0x28, 0xE0, 0xE7, 0x14, 0xC0, 0x42, 0x89, 0x02, 0x01, 0x11,
};

steam::CMClient::CMClient(boost::asio::io_service& io) :
    _io(io),
    _handshakeComplete(false),
    _socket(std::make_unique<boost::asio::ip::tcp::socket>(io)),
    _heartbeat(io)
{
    _heartbeat.SetCallback([this](auto ec) { this->SendHeartbeat(ec); });
	IncomingPacket.connect([this](EMsg emsg, const uint8_t* data, std::size_t length, uint64_t jobid)
	{
		this->HandleMessage(emsg, data, length, jobid);
	});
}

void steam::CMClient::Connect(boost::asio::ip::tcp::resolver::iterator& endpoint)
{
    boost::asio::async_connect(*_socket, endpoint,
		[&](const boost::system::error_code& ec,
            boost::asio::ip::tcp::resolver::iterator)
	{
		if (ec)
		{
			Disconnect(ec);
			return;
		}
        ReadPacketHeader();
	});
}

void steam::CMClient::ReadPacketHeader()
{
	// Each Packet has a 8 byte header with VT01[packet length]
	_readBuffer.resize(std::max<std::size_t>(8, _readBuffer.size()), '\0');
    _socket->async_receive(boost::asio::buffer(this->_readBuffer.data(), 8),
		[&](boost::system::error_code ec, std::size_t)
	{
		if (ec)
		{
			Disconnect(ec);
			return;
		}

        uint32_t length = *reinterpret_cast<uint32_t*>(_readBuffer.data());
        _readOffset = 0;
        _readBuffer.resize(std::max<std::size_t>(length, _readBuffer.size()), '\0');
        ReadPacket(length, length);
	});
}

void steam::CMClient::ReadPacket(uint32_t remainingSize, uint32_t expectedSize)
{
    _socket->async_receive(boost::asio::buffer(_readBuffer.data() + _readOffset, expectedSize - _readOffset),
        [this, remainingSize, expectedSize](boost::system::error_code ec, std::size_t length)
    {
        if (ec)
        {
            Disconnect(ec);
            return;
        }

        uint32_t remaining = remainingSize - length;
        if (remaining == 0)
        {
            DecryptAndHandle(expectedSize);

            // Read next packet
            ReadPacketHeader();
        }
        else
        {
            _readOffset += length;
            // Continue reading this packet
            ReadPacket(remaining, expectedSize);
        }
    });
}


void steam::CMClient::Disconnect()
{
	Disconnect(boost::system::error_code());
}

bool steam::CMClient::IsConnected()
{
	return _handshakeComplete;
}

void steam::CMClient::WriteMessage(EMsg emsg, const std::vector<uint8_t>& input)
{
    if (emsg == EMsg::ChannelEncryptResponse)
    {
        // ChannelEncryptResponse uses a small header
        auto data = std::vector<uint8_t>(sizeof(MsgHdr) + input.size());
        auto header = new (data.data()) MsgHdr;
        header->msg = emsg;
        std::copy(input.data(), input.data() + input.size(), data.data() + sizeof(MsgHdr));
        WritePacket(data);
    }
    else
    {
        // everything else uses an extended header with sessionID and steamID
        auto data = std::vector<uint8_t>(sizeof(ExtendedClientMsgHdr) + input.size());
        auto header = new (data.data()) ExtendedClientMsgHdr;
        header->msg = emsg;
        header->sessionID = _sessionID;
        header->steamID = steamID;
        std::copy(input.data(), input.data() + input.size(), data.data() + sizeof(ExtendedClientMsgHdr));
        WritePacket(data);
    }
}

void steam::CMClient::WriteMessage(EMsg emsg, const google::protobuf::Message &message, std::uint64_t job_id, uint32_t appid) {
    proto::steam::CMsgProtoBufHeader proto;
    proto.set_steamid(steamID);
    proto.set_client_sessionid(_sessionID);

    if (appid) {
        proto.set_routing_appid(appid);
    }

    if (job_id) {
        proto.set_jobid_target(job_id);
    }

    auto proto_size = proto.ByteSize();
    auto message_size = message.ByteSize();

    auto input = std::vector<uint8_t>(sizeof(MsgHdrProtoBuf) + proto_size + message_size);
    auto header = new (input.data()) MsgHdrProtoBuf;
    header->headerLength = proto_size;
    header->msg = static_cast<steam::EMsg>(static_cast<std::uint32_t>(emsg) | PROTO_MASK);
    proto.SerializeToArray(input.data() + sizeof(MsgHdrProtoBuf), proto_size);
    message.SerializeToArray(input.data() + sizeof(MsgHdrProtoBuf) + proto_size, message_size);

    WritePacket(input);
}

void steam::CMClient::WritePacket(const std::vector<uint8_t>& input)
{
    auto length = input.size();

    // For unencrypted packets, the packet length is the data length
    auto headerLength = length;

    if (_handshakeComplete) 
    {
        // For encrypted packets we need to add 16 bytes for the IV
        // and pad the message to a block of 16 bytes
        headerLength = 16 + (length / 16 + 1) * 16;
    }

    // Setup Packet Header
    auto buffer = new uint8_t[headerLength + 8];
    *reinterpret_cast<std::uint32_t*>(buffer) = headerLength;
    const char* MAGIC = "VT01";
    std::copy(MAGIC, MAGIC + 4, buffer + 4);

    // Encrypt the packet if needed
    if (_handshakeComplete) {
        using namespace CryptoPP;
        
        // Generate a random IV
        uint8_t iv[16];
        _rng.GenerateBlock(iv, sizeof(iv));

        // Encrypt the IV via the session key
        ECB_Mode<AES>::Encryption ecb(_sessionKey, sizeof(_sessionKey));
        ecb.ProcessData(buffer + 8, iv, sizeof(iv));

        // Encrypt the data using the session key and the IV
        CBC_Mode<AES>::Encryption cbc(_sessionKey, sizeof(_sessionKey), iv);
        ArraySource(
            input.data(),
            input.size(),
            true,
            new StreamTransformationFilter(cbc, new ArraySink(buffer + 8 + sizeof(iv), headerLength - sizeof(iv)))
        );
    }
    else 
    {
		// The initial handshake is not encrypted, simply copy the data
        std::copy(input.data(), input.data() + length, buffer + 8);  
    }

    // Send it
    _socket->async_send(boost::asio::buffer(buffer, headerLength + 8),
        [buffer, this](boost::system::error_code ec, std::size_t)
    {
        delete[] buffer;
        if (ec)
        {
            Disconnect(ec);
            return;
        }
    });
}


void steam::CMClient::Disconnect(const boost::system::error_code & ec)
{
	_socket = std::make_unique<boost::asio::ip::tcp::socket>(_io);
	_handshakeComplete = false;
	_readBuffer.clear();
	_sessionID = 0;
	Disconnected(ec);
}

void steam::CMClient::DecryptAndHandle(std::size_t length) {
	// The initial handshake is not encrypted
	using namespace CryptoPP;
	if (_handshakeComplete) {
        // Decrypt IV
        uint8_t iv[16];
        ECB_Mode<AES>::Decryption ecb(_sessionKey, sizeof(_sessionKey));
        ecb.ProcessData(iv, _readBuffer.data(), sizeof(iv));

        // Decrypt data
		CBC_Mode<AES>::Decryption cbc(_sessionKey, sizeof(_sessionKey), iv);
		std::string output;
		ArraySource(
            _readBuffer.data() + sizeof(iv),
			length - sizeof(iv),
			true,
			new StreamTransformationFilter(cbc, new StringSink(output))
		);

		HandlePacket(reinterpret_cast<const uint8_t*>(output.data()), output.length());
	}
	else
	{
		HandlePacket(_readBuffer.data(), length);
	}
}

void steam::CMClient::HandlePacket(const uint8_t* data, std::size_t length)
{
	auto raw_emsg = *reinterpret_cast<const std::uint32_t*>(data);
	auto emsg = static_cast<EMsg>(raw_emsg & ~PROTO_MASK);

	// Initial handshake uses MsgHdr and is not encrypted
	if (emsg == EMsg::ChannelEncryptRequest || emsg == EMsg::ChannelEncryptResult) {
		auto header = reinterpret_cast<const MsgHdr*>(data);
		IncomingPacket(emsg, data + sizeof(MsgHdr), length - sizeof(MsgHdr), header->sourceJobID);
	}
	else if (raw_emsg & PROTO_MASK) 
    {
		// Messages with the protobfu mask use a MsgHdrProtobuf
		auto header = reinterpret_cast<const MsgHdrProtoBuf*>(data);
		proto::steam::CMsgProtoBufHeader proto;
		proto.ParseFromArray(data + sizeof(MsgHdrProtoBuf), header->headerLength);
		// In a login message we receive our own session id and steamid
		// The session id is needed for sending messages
		if (!_sessionID && header->headerLength > 0) {
			_sessionID = proto.client_sessionid();
			steamID = proto.steamid();
		}
		IncomingPacket(
			emsg,
			data + sizeof(MsgHdrProtoBuf) + header->headerLength,
			length - sizeof(MsgHdrProtoBuf) - header->headerLength,
			proto.jobid_source()
		);
	}
	else 
    {
		// Other non-protobuf messages use an extended header
        auto header = reinterpret_cast<const ExtendedClientMsgHdr*>(data);
        IncomingPacket(emsg, data + sizeof(ExtendedClientMsgHdr), length - sizeof(ExtendedClientMsgHdr), header->sourceJobID);
    }
}


void steam::CMClient::HandleMessage(EMsg emsg, const uint8_t* data, std::size_t length, uint64_t /*job_id*/) {
	// We only handle a few messages here, since CMClient's purpose is to connect and log on to steam
	// without doing much else. It also handles unpacking multi messages into single messages.
	switch (emsg)
	{
	case EMsg::ChannelEncryptRequest:
		return HandleEncryptRequest(data, length);
	case EMsg::ChannelEncryptResult:
		return HandleEncryptResponse(data, length);
	case EMsg::Multi:
		return HandleMulti(data, length);
	case EMsg::ClientLogOnResponse:
		return HandleLogOnResponse(data, length);
	case EMsg::ClientLoggedOff:
		return HandleLoggedOff(data, length);
	}
}

void steam::CMClient::HandleEncryptRequest(const uint8_t*, std::size_t)
{
	// Initial handshake:
	// -> User connects to Steam
	// <- Steam requests encryption
	// -> User generates a random encryption key for the session
	// -> User encrypts the session key with steam's public key
	// -> User CRC32's the encrypted payload and sends it to steam
	// <- Steam indicates handshake success
	using namespace CryptoPP;

	RSA::PublicKey key;
	ArraySource source(steamPublicKey, sizeof(steamPublicKey), true);
	key.Load(source);
	RSAES_OAEP_SHA_Encryptor rsa(key);

	auto rsaLength = rsa.FixedCiphertextLength();

    std::vector<uint8_t> response(sizeof(MsgChannelEncryptResponse) + rsaLength + 8);
    new (response.data()) MsgChannelEncryptResponse;
    auto sessionKeyRSA = response.data() + sizeof(MsgChannelEncryptResponse);

    _rng.GenerateBlock(_sessionKey, sizeof(_sessionKey));

    rsa.Encrypt(_rng, _sessionKey, sizeof(_sessionKey), sessionKeyRSA);

    CRC32().CalculateDigest(sessionKeyRSA + rsaLength, sessionKeyRSA, rsaLength);
    *reinterpret_cast<uint32_t*>(sessionKeyRSA + rsaLength + 4) = 0;

    WriteMessage(EMsg::ChannelEncryptResponse, response);
}

void steam::CMClient::HandleEncryptResponse(const uint8_t* data, std::size_t)
{
	const auto& msg = *reinterpret_cast<const MsgChannelEncryptResult*>(data);
	if (msg.result == EResult::OK)
	{
		_handshakeComplete = true;
		Connected();
	}
}

void steam::CMClient::HandleMulti(const uint8_t * data, std::size_t length)
{
	// A multi packet is a message containing multiple other messages
	// in an inflated memory payload. Simply unpack them and handle them 
	// as if they were sent normally
	proto::steam::CMsgMulti multi;
	multi.ParseFromArray(data, length);

    auto body = multi.message_body();
    std::vector<uint8_t> msgdata = std::vector<uint8_t>(body.begin(), body.end());

    // If neccessary, decompress
	if (multi.has_size_unzipped() && multi.size_unzipped() != 0)
	{
        auto packed = msgdata;
        msgdata.resize(multi.size_unzipped(), '\0');

		if (!Inflate(packed, msgdata))
			return;
	}

	auto payload_size = msgdata.size();
	for (unsigned offset = 0; offset < payload_size;) {
		auto subSize = *reinterpret_cast<const std::uint32_t*>(msgdata.data() + offset);
		HandlePacket(msgdata.data() + offset + 4, subSize);
		offset += 4 + subSize;
	}
}

void steam::CMClient::SendHeartbeat(const boost::system::error_code&)
{
	// If we disconnected, stop sending heartbeats
	if (!IsConnected())
		return;

	WriteMessage(EMsg::ClientHeartBeat, proto::steam::CMsgClientHeartBeat());
}

void steam::CMClient::HandleLogOnResponse(const uint8_t * data, std::size_t length)
{
	// Handle logon responses, if logging in succeeded we need to send heartbeat messages
	proto::steam::CMsgClientLogonResponse logon_resp;
	logon_resp.ParseFromArray(data, length);
	LoggedOn(logon_resp);
    auto eresult = static_cast<EResult>(logon_resp.eresult());
	if (eresult == EResult::OK) {
        _heartbeat.Start(std::chrono::seconds(logon_resp.out_of_game_heartbeat_seconds()));
	}
}

void steam::CMClient::HandleLoggedOff(const uint8_t * data, std::size_t length)
{
	// If we receive a logged off packet, call the event and stop sending cm heartbeats
	proto::steam::CMsgClientLoggedOff logged_off;
	logged_off.ParseFromArray(data, length);
	LoggedOff(logged_off);
    _heartbeat.Stop();
}

bool steam::CMClient::Inflate(const std::vector<uint8_t>& input, std::vector<uint8_t>& output)
{
    z_stream zstrm;

    zstrm.zalloc = Z_NULL;
    zstrm.zfree = Z_NULL;
    zstrm.opaque = Z_NULL;
    zstrm.avail_in = 0;
    zstrm.next_in = Z_NULL;

    int ret = inflateInit2(&zstrm, 16 + MAX_WBITS);

    if (ret != Z_OK)
        return false;

    zstrm.avail_in = input.size();
    zstrm.next_in = const_cast<Bytef *>(input.data());

    zstrm.avail_out = output.size();
    zstrm.next_out = output.data();

    ret = inflate(&zstrm, Z_NO_FLUSH);

    inflateEnd(&zstrm);

    return ret == Z_STREAM_END;
}