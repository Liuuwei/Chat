#include "ChatService.h"
#include "Public.h"



#include <muduo/base/Logging.h>

#include <vector>

using namespace muduo;

ChatService* ChatService::instance() {
    static ChatService service;
    return &service;
}

ChatService::ChatService() {
    messageHandlerMap_.insert({LOGIN, std::bind(&ChatService::login, this, _1, _2, _3)});
    messageHandlerMap_.insert({REGIS, std::bind(&ChatService::regis, this, _1, _2, _3)});
    messageHandlerMap_.insert({ONE_CHAT, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    messageHandlerMap_.insert({ADD_FRI, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    messageHandlerMap_.insert({CREATE_GROUP, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    messageHandlerMap_.insert({JOIN_GROUP, std::bind(&ChatService::joinGroup, this, _1, _2, _3)});
    messageHandlerMap_.insert({GROUP_CHAT, std::bind(&ChatService::groupChat, this, _1, _2, _3)});
}

MessageHandler ChatService::getHandler(int msgId) {
    auto it = messageHandlerMap_.find(msgId);
    if (it == messageHandlerMap_.end()) {
        return [=](auto a, auto b, auto c) {
            LOG_ERROR << "msgid: " << msgId << " can't find handler!";
        };
    } else {
        return messageHandlerMap_[msgId];
    }
}

void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int id = js["id"].get<int>();
    std::string password = js["password"];

    User user = userModel_.query(id);

    json response;
    response["msgid"] = LOGIN_ACK;
    if (user.getId() != -1 && user.getPassword() == password) {
        if (user.getState() == "online") {
            response["errno"] = 1;
            response["errmsg"] = "Account is online";
        } else { // * 登录成功
            {
                std::lock_guard lock(connMutex_);
                userConnectionMap_.insert({user.getId(), conn});
            }
            response["errno"] = 0;
            // * 更新状态信息
            user.setState("online");
            userModel_.updateState(user);

            std::vector<std::string> msgVec = offlineMessageModel_.query(id);
            std::vector<User> userVec = friendModel_.query(id);
            std::vector<Group> groupVec = groupModel_.queryGroups(id);

            if (msgVec.size()) {
                response["offlinemessage"] = msgVec;
                offlineMessageModel_.remove(id);
            }

            std::vector<std::string> userVec2;
            if (userVec.size()) {
                for (auto &user : userVec) {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    userVec2.push_back(js.dump());
                }
                response["friend"] = userVec2;
            }

            std::vector<std::string> groupStr;
            if (groupVec.size()) {
                for (auto &group : groupVec) {
                    json js;
                    js["groupid"] = group.getId();
                    js["groupname"] = group.getName();
                    js["groupdesc"] = group.getDesc();
                    groupStr.push_back(js.dump());
                }
                response["group"] = groupStr;
            }
        }
    } else {
        response["errno"] = 1;
        response["errmsg"] = "id or password is error";
    }
    conn->send(response.dump());
}

void ChatService::regis(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    std::string name = js["name"];
    std::string password = js["password"];

    User user(name, password);

    json response;
    response["msgid"] = REGIS_ACK;
    if (userModel_.insert(user)) {
        response["id"] = user.getId();
        response["errno"] = 0;
    } else {
        response["id"] = -1;
        response["errno"] = 1;
        response["errmsg"] = "registration failed";
    }
    conn->send(response.dump());
}

void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int toId = js["to"].get<int>();
    {
        std::lock_guard lock(connMutex_);
        auto it = userConnectionMap_.find(toId);
        if (it != userConnectionMap_.end()) { // * to is online
            it->second->send(js.dump());
        } else {
            offlineMessageModel_.insert(toId, js.dump()); // * save offline message
        }
    }
}

void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int userId = js["id"].get<int>();
    int friendId = js["friendid"].get<int>();
    friendModel_.insert(userId, friendId);
}

void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int userId = js["id"];
    std::string name = js["groupname"];
    std::string desc = js["groupdesc"];
    Group group(-1, name, desc);
    if (groupModel_.createGroup(group)) {
        groupModel_.joinGroup(userId, group.getId(), "creator");
    }
}

void ChatService::joinGroup(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int userId = js["id"];
    int groupId = js["groupId"];
    groupModel_.joinGroup(userId, groupId, "normal");
}

void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int userId = js["id"];
    int groupId = js["groupid"];
    std::vector<int> groupUser = groupModel_.queryGroupUsers(userId, groupId);
    
    std::lock_guard lock(connMutex_);
    for (auto id : groupUser) {
        auto it = userConnectionMap_.find(id);
        if (it != userConnectionMap_.end()) {
            it->second->send(js.dump());
        } else {
            offlineMessageModel_.insert(id, js.dump());
        }
    }
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn) {
    User user;
    user.setState("offline");
    {
        std::lock_guard lock(connMutex_);
        for (auto it = userConnectionMap_.begin(); it != userConnectionMap_.end(); ++ it) {
            if (it->second == conn) {
                user.setId(it->first);
                userConnectionMap_.erase(it);
                break;
            }
        }
    }
    if (user.getId() != -1)
        userModel_.updateState(user);
}

void ChatService::resetState() {
    userModel_.resetState();
}