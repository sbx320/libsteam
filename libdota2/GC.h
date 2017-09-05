#pragma once
#include "Client.h"

// Forward declacations to speed up compilation
namespace proto
{
    namespace dota {
        enum EGCBaseClientMsg : int;
        enum EDOTAGCMsg : int;
        enum EGCBaseMsg : int;
    }
}

namespace steam
{
	class GC : public Client
	{
	public:
		GC(boost::asio::io_service& io, uint32_t appId) : Client(io), _appId(appId)
		{
			IncomingPacket.connect([this](EMsg emsg, const uint8_t* data, std::size_t length, uint64_t jobid)
			{
				HandleMessage(emsg, data, length, jobid);
			});
		}
		void SendGC(proto::dota::EGCBaseClientMsg emsg, const google::protobuf::Message& message);
		void SendGC(proto::dota::EGCBaseMsg emsg, const google::protobuf::Message& message)
		{
			return SendGC(static_cast<proto::dota::EGCBaseClientMsg>(emsg), message);
		}
		void SendGC(proto::dota::EDOTAGCMsg emsg, const google::protobuf::Message& message)
		{
			return SendGC(static_cast<proto::dota::EGCBaseClientMsg>(emsg), message);
		}

		ksignals::Event<void(uint32_t, const uint8_t*, std::size_t, uint64_t)> GCMessage;
	private:
		void HandleMessage(EMsg emsg, const uint8_t* data, std::size_t length, uint64_t jobid);
	protected:
		uint32_t _appId;

	};
}
