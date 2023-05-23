#include "Redis.h"

#include <stdio.h>

#include <string>

Redis::Redis() : publish_context_(nullptr), subscribe_context_(nullptr) {

}

Redis::~Redis() {
    if (publish_context_)
        redisFree(publish_context_);
    if (subscribe_context_)
        redisFree(subscribe_context_);
}

bool Redis::connect() {
    publish_context_ = redisConnect("127.0.0.1", 6379);
    if (publish_context_ == nullptr) {
        printf("connect redis failed\n");
        return false;
    }
    subscribe_context_ = redisConnect("127.0.0.1", 6379);
    if (subscribe_context_ == nullptr) {
        printf("connect redis failed\n");
        return false;
    }

    std::thread t([&]() {
        observer_channel_message();
    });
    t.detach();

    printf("connect redis-server success\n");
    return true;
}

bool Redis::publish(int channel, const std::string &message) {
    redisReply *reply = (redisReply *)redisCommand(publish_context_, "PUBLISH %d %s", channel, message.c_str());
    if (reply == nullptr) {
        printf("publish command failed\n");
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool Redis::subscribe(int channel) {
    if (redisAppendCommand(subscribe_context_, "SUBSCRIBE %d", channel) == REDIS_ERR) {
        printf("subscribe command failed\n");
        return false;
    }
    int done = 0;
    while (!done) {
        if (redisBufferWrite(subscribe_context_, &done) == REDIS_ERR) {
            printf("subscribe command failed\n");
            return false;
        }
    }
    return true;
}

bool Redis::unsubscribe(int channel) {
    if (redisAppendCommand(subscribe_context_, "UNSUBSCRIBE %d", channel) == REDIS_ERR) {
        printf("subscribe command failed\n");
        return false;
    }
    int done = 0;
    while (!done) {
        if (redisBufferWrite(subscribe_context_, &done) == REDIS_ERR) {
            printf("subscribe command failed\n");
            return false;
        } 
    }
    return true;
}

void Redis::observer_channel_message() {
    redisReply *reply = nullptr;
    while (redisGetReply(this->subscribe_context_, (void **)&reply) == REDIS_OK) {
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr) {
            notify_message_handler(atoi(reply->element[1]->str), reply->element[2]->str);
        }
        freeReplyObject(reply);
    }
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
}

void Redis::init_notify_handler(std::function<void(int, std::string)> fn) {
    notify_message_handler = fn;
}