#pragma once
#include "CMClient.h"

namespace steam {
	class Client : public CMClient
	{
	public:
        Client(boost::asio::io_service& io);

		void LogOn(const std::string& username, const std::string& password);

		void SetPersonaState(EPersonaState state);
	};
}