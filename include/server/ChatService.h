#pragma once

#include "OfflineMessageModel.h"
#include "GroupModel.h"
#include "FriendModel.h"
#include "UserModel.h"
#include "Redis.h"
#include "json.hpp"

#include <muduo/net/TcpConnection.h>

#include <unordered_map>
#include <functional>
#include <mutex>

using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

using MessageHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp time)>;

class ChatService {
    public:
        static ChatService* instance();
        void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
        void regis(const TcpConnectionPtr &conn, json &js, Timestamp time);
        void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
        void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
        void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
        void joinGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
        void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
        void showFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
        void showGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
        void clientCloseException(const TcpConnectionPtr &conn);
        void handleRedisSubscribeMessage(int ,const std::string&);
        void resetState();
        void resetRedis();
        MessageHandler getHandler(int msgId);
    private:
        ChatService();
        Redis redis_;
        UserModel userModel_;
        FriendModel friendModel_;
        GroupModel groupModel_;
        OfflineMessageModel offlineMessageModel_;
        std::mutex connMutex_;
        std::unordered_map<int, MessageHandler> messageHandlerMap_;
        std::unordered_map<int, const TcpConnectionPtr &> userConnectionMap_;
};