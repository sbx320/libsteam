#pragma once
#include "CMClient.h"

namespace steam {
	class Client : public CMClient
	{
	public:
        Client(net::io_context& io);

		void LogOn(const std::string& username, const std::string& password);

		void SetPersonaState(EPersonaState state);
	};
}