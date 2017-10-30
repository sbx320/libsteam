#pragma once
#include <experimental/net>
#include <chrono>
#include <functional>
#include <memory>

namespace steam
{
    class RecurringTimer
    {
    public:
        using timer_t = std::experimental::net::basic_waitable_timer<std::chrono::steady_clock>;
        using duration_t = std::chrono::steady_clock::duration;
        using func_t = std::function<void(std::error_code)>;
        RecurringTimer(std::experimental::net::io_context& io);

        void SetCallback(func_t func);
        void Start(const duration_t& interval);
        void Stop();
        bool Running() { return _timer.get() != nullptr; }

    private:
        void Expired(const std::error_code& ec);

        func_t _func;
        duration_t _interval;
		std::experimental::net::io_context& _io;
        std::unique_ptr<timer_t> _timer;
    };
}