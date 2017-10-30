#include "RecurringTimer.h"
#include "NetTS.h"

using namespace steam;

RecurringTimer::RecurringTimer(net::io_context & io) : _io(io)
{
}

void RecurringTimer::SetCallback(func_t func)
{
    _func = func;
}

void RecurringTimer::Start(const duration_t& interval)
{
    if (_timer)
        return;

    _interval = interval;
        
    _timer = std::make_unique<net::steady_timer>(_io, _interval);
    _timer->expires_after(_interval);
    _timer->async_wait([&](const std::error_code& ec) { Expired(ec); });
}


void RecurringTimer::Stop()
{
    _timer.reset();
}

void RecurringTimer::Expired(const std::error_code & ec)
{
    if (!_timer)
        return;

    _func(ec);
    _timer->expires_after(_interval);
    _timer->async_wait([&](const std::error_code& ec) { Expired(ec); });
}
