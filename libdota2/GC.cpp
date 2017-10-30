#include "GC.h"
#include <steam/steammessages_base.pb.h>
#include <dota/dota_gcmessages_client.pb.h>
#include <dota/dota_gcmessages_msgid.pb.h>
#include <dota/gcsystemmsgs.pb.h>
#include <dota/base_gcmessages.pb.h>
#include <steam/encrypted_app_ticket.pb.h>
#include <steam/steammessages_clientserver.pb.h>
#include <steam/steammessages_clientserver_2.pb.h>

void steam::GC::SendGC(proto::dota::EGCBaseClientMsg emsg, const google::protobuf::Message & message)
{
	proto::steam::CMsgProtoBufHeader proto;
	proto.set_steamid(steamID);
	proto.set_client_sessionid(_sessionID);
	proto.set_routing_appid(_appId);

	auto protoSize = proto.ByteSize();
	auto messageSize = message.ByteSize();
    std::vector<uint8_t> buffer;
	buffer.resize(sizeof(MsgGCHdrProtoBuf) + protoSize + messageSize);
	auto header = new (buffer.data()) MsgGCHdrProtoBuf;
	header->headerLength = protoSize;
	header->msg = static_cast<std::uint32_t>(emsg) | PROTO_MASK;
	proto.SerializeToArray(buffer.data() + sizeof(MsgGCHdrProtoBuf), protoSize);
	message.SerializeToArray(buffer.data() + sizeof(MsgGCHdrProtoBuf) + protoSize, messageSize);

	proto::steam::CMsgGCClient packed;
	packed.set_appid(_appId);
	packed.set_msgtype(emsg | PROTO_MASK);
	packed.set_payload(buffer.data(), buffer.size());
	WriteMessage(EMsg::ClientToGC, packed, 0, _appId);
}

void steam::GC::HandleMessage(EMsg emsg, const uint8_t * data, std::size_t length, uint64_t jobid)
{
	if (emsg != EMsg::ClientFromGC)
		return;

	proto::steam::CMsgGCClient msg;
	msg.ParseFromArray(data, length);
	if (msg.appid() != _appId)
		return;

	auto gcemsg = msg.msgtype() & ~PROTO_MASK;

	auto& payload = msg.payload();
	auto& header = *(MsgGCHdrProtoBuf*)payload.data();
	auto protoSize = header.headerLength;

	GCMessage(gcemsg, reinterpret_cast<const uint8_t*>(payload.data()) + sizeof(MsgGCHdrProtoBuf) + protoSize, length - sizeof(MsgGCHdrProtoBuf) - protoSize, jobid);
}
