#include "ChatServer.h"
#include "ChatService.h"
#include "json.hpp"

#include <functional>
#include <iostream>
#include <string>

using namespace std::placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop, const InetAddress &addr, const std::string &nameArg) :
    loop_(loop), 
    server_(loop, addr, nameArg) {
    server_.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));
    server_.setThreadNum(4);
}

void ChatServer::start() {
    server_.start();
}

void ChatServer::onConnection(const TcpConnectionPtr &conn) {
    if (!conn->connected()) {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }

}

void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp receiveTime) {
    std::string buf = buffer->retrieveAllAsString();
    json js = json::parse(buf);
    
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    msgHandler(conn, js, receiveTime);
}