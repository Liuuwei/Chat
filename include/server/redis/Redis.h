#pragma once

#include <hiredis/hiredis.h>
#include <thread>
#include <functional>

class Redis {
    public:
        Redis();
        ~Redis();
        bool connect();
        bool publish(int channel, const std::string &msg);
        bool subscribe(int channel);
        bool unsubscribe(int channel);
        void observer_channel_message();
        void init_notify_handler(std::function<void(int, std::string)> fn);
    private:
        redisContext *publish_context_;
        redisContext *subscribe_context_;
        std::function<void(int, std::string)> notify_message_handler;
};