#include "json.hpp"
#include "Public.h"
#include "Group.h"
#include "User.h"

#include <muduo/base/Timestamp.h>

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <functional>
#include <iostream>
#include <vector>
#include <thread>

using json = nlohmann::json;
using namespace muduo;

int clientfd;

// * 当前登录用户
User g_currentUser;
// * 好友列表
std::vector<User> g_currentUserFriendList;
// * 群组列表
std::vector<Group> g_currentUserGroupList;
// * 显示用户信息
void showCurrentUserData(json &js);

void send(json &js);

void mainMenu();
void funMenu();

bool login();
void regis();

void readFun();

void oneChat();
void addFriend();
void createGroup();
void joinGroup();
void chatGroup();

std::unordered_map<std::string, std::function<void()>> funMap {
    {"chat", oneChat},
    {"addfriend", addFriend},
    {"createGroup", createGroup},
    {"chatgroup", chatGroup}
};


int main() {

    const char *ip = "10.211.55.3";
    int port = 9999;
    sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &addr.sin_addr);
    
    clientfd = socket(AF_INET, SOCK_STREAM, 0);
    int ret = ::connect(clientfd, (sockaddr *)&addr, sizeof(addr));
    assert(ret != -1);

    mainMenu();
}

void mainMenu() {
    while(true) {
        int choice;
        printf("1:login\n2:regis\n3:quit\n");
        std::cin >> choice;
        std::cin.get();
        switch(choice) {
            case 1: {
                if (login()) {
                    std::thread readThread(readFun);
                    readThread.detach();

                    funMenu();
                }
                break;
            }
            case 2: {
                regis();
                break;
            }
            case 3: {
                exit(0);
            }
            default: {
                break;
            }
        }
    }
}

void funMenu() {
    while (true) {
        std::string method;
        printf("enter:");
        std::cin >> method;
        auto fun = funMap.find(method);
        if (fun == funMap.end()) {
            printf("again\n");
            continue;
        }
        fun->second();
    }
}

bool login() {
    int id;
    printf("userid:");
    std::cin >> id;
    std::string password;
    printf("password:");
    std::cin >> password;
    json js;
    js["msgid"] = 1;
    js["id"] = id;
    js["password"] = password;
    std::string str = js.dump();
    int len = ::send(clientfd, str.c_str(), str.size(), 0);

    char buf[1024] = {0};
    len = ::recv(clientfd, buf, sizeof(buf), 0);
    std::string msg = buf;
    std::cout << msg << std::endl; //out!
    json res = json::parse(msg);
    if (res["errno"] != 0) {
        printf("%s\n", res["errmsg"].dump().c_str());
        return false;
    } 
    g_currentUser.setId(res["id"].get<int>());
    g_currentUser.setName(res["name"].dump());

    if (res.contains("friend")) {
        std::vector<std::string> friends = res["friend"];
        for (auto &fri : friends) {
            User user;
            json frijs = json::parse(fri);
            user.setId(frijs["id"].get<int>());
            user.setName(frijs["name"]);
            user.setState(frijs["state"]);
            g_currentUserFriendList.push_back(user);
        }
    }

    if (res.contains("group")) {
        std::vector<std::string> groups = res["group"];
        for (std::string &group : groups) {
            Group grou;
            json grojs = json::parse(group);
            grou.setId(grojs["groupid"]);
            grou.setName(grojs["groupname"]);
            grou.setDesc(grojs["groupdesc"]);
            g_currentUserGroupList.push_back(grou);
        }
    }

    showCurrentUserData(res);

    return true;
}

void regis() {
    printf("username:");
    std::string name;
    std::getline(std::cin, name);
    std::string password;
    std::cin >> password;
    json js;
    js["msgid"] = 3;
    js["name"] = name;
    js["password"] = password;
    send(js);
}

void showCurrentUserData(json &js) {
    printf("==========login==========\n");
    printf("current login user ==>> id:%d name:%s\n", g_currentUser.getId(), g_currentUser.getName().c_str());
    printf("----------friendlist----------\n");
    if (g_currentUserFriendList.size()) {
        for (User &fri : g_currentUserFriendList) {
            printf("%d %s %s\n", fri.getId(), fri.getName().c_str(), fri.getState().c_str());
        }
    }   
    printf("----------grouplist----------\n");
    if (g_currentUserGroupList.size()) {
        for (Group &gro : g_currentUserGroupList) {
            printf("%s:%s\n", gro.getName().c_str(), gro.getDesc().c_str());
        }
    }
    printf("----------offlinemessage----------\n");
    if (js.contains("offlinemessage")) {
        std::vector<std::string> offlinemessage = js["offlinemessage"];
        for (std::string &msg : offlinemessage) {
            std::cout << msg << std::endl;
            json offjs = json::parse(msg);
            printf("%s:%s:%s\n", offjs["time"].dump().c_str(), offjs["name"].dump().c_str(), offjs["msg"].dump().c_str());
        }
    }
}

void oneChat() {
    int toId;
    std::string msg;
    std::cin >> toId;
    std::cin.get();
    std::getline(std::cin, msg);
    json js;
    js["msgid"] = 5;
    js["toid"] = toId;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["time"] = Timestamp::now().toFormattedString();
    js["msg"] = msg;
    send(js);
}

void addFriend() {
    int toId;
    std::cin >> toId;
    std::cin.get();
    json js;
    js["msgid"] = 6;
    js["toid"] = toId;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["time"] = Timestamp::now().toFormattedString();
    send(js);
}

void createGroup() {
    std::string groupName, groupDesc;
    std::getline(std::cin, groupName);
    std::getline(std::cin, groupDesc);
    json js;
    js["msgid"] = 7;
    js["id"] = g_currentUser.getId();
    js["time"] = Timestamp::now().toFormattedString();
    js["groupname"] = groupName;
    js["groupdesc"] = groupDesc;
    send(js);
}

void joinGroup() {
    int toId;
    std::cin >> toId;
    std::cin.get();
    json js;
    js["msgid"] = 8;
    js["toid"] = toId;
    js["id"] = g_currentUser.getId();
    send(js);
}

void chatGroup() {
    int toId;
    std::cin >> toId;
    std::cin.get();
    std::string msg;
    std::getline(std::cin, msg);
    json js;
    js["msgid"] = 9;
    js["toId"] = toId;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["msg"] = msg;
    send(js);
}

void send(json &js) {
    std::string msg = js.dump();
    int len = ::send(clientfd, msg.c_str(), msg.size(), 0);
    if (len == -1) {
        std::cerr << "send message failed\n";
    }
}

void readFun() {
    char buf[1024] = {0};
    while (true) {
        int len = ::recv(clientfd, buf, sizeof(buf), 0);
        if (len == -1) {
            std::cerr << "recv message failed\n";
            return ;
        }
        std::string msg = buf;
        json js = json::parse(msg);
        printf("%s:%s:%s\n", js["time"].dump().c_str(), js["name"].dump().c_str(), js["msg"].dump().c_str());
    }
}