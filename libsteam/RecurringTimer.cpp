#include "RecurringTimer.h"
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

using namespace steam;

RecurringTimer::RecurringTimer(boost::asio::io_service & io) : _io(io)
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
        
    _timer = std::make_unique<boost::asio::steady_timer>(_io, interval);
    _timer->expires_from_now(interval);
    _timer->async_wait([&](const boost::system::error_code& ec) { Expired(ec); });
}


void RecurringTimer::Stop()
{
    _timer.reset();
}

void RecurringTimer::Expired(const boost::system::error_code & ec)
{
    if (!_timer)
        return;

    _func(ec);
    _timer->expires_from_now(_interval);
    _timer->async_wait([&](const boost::system::error_code& ec) { Expired(ec); });
}
