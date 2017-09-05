#pragma once

#include <vector>
#include <algorithm>
#include <utility>
#include <functional>

template<typename... Args>
class CallbackFunc
{
    using func_t = void(Args...);
    std::vector<std::function<func_t>> _callbacks;
public:
    template<typename T>
    void operator+=(T callable)
    {
        _callbacks.push_back(std::function<func_t>(callable));
    }

    void operator+=(func_t callable)
    {
        _callbacks.push_back(callable);
    }

    void operator()(Args... args) const
    {
        for (auto& callback : _callbacks)
            callback(args...);
    }

};

template<typename T>
using CallbackManager = CallbackFunc<const T&>;
