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
#include <condition_variable>
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>

using json = nlohmann::json;
using namespace muduo;

std::mutex mutex;
std::condition_variable cond;

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

json logjs;

void chat(json &js);
void showfriend(json &js);
void showgroup(json &js);
void showregis(json &js);

void oneChat();
void addFriend();
void createGroup();
void joinGroup();
void chatGroup();
void showFriend();
void showGroup();

std::unordered_map<std::string, std::function<void()>> funMap {
    {"chat", oneChat},
    {"addfriend", addFriend},
    {"joingroup", joinGroup},
    {"creategroup", createGroup},
    {"chatgroup", chatGroup},
    {"showfriend", showFriend},
    {"showgroup", showGroup}
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

    std::thread readThread(readFun);
    readThread.detach();

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
        std::cin >> method;
        std::cin.get();
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
    json res;

    {
        std::unique_lock lock(mutex);
        cond.wait(lock);
        res = logjs;
    }
    if (res["errno"].get<int>() != 0) {
        printf("%s\n", res["errmsg"].dump().c_str());
        return false;
    } 
    g_currentUser.setId(res["id"].get<int>());
    g_currentUser.setName(res["name"]);

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
    printf("password:");
    std::string password;
    std::cin >> password;
    json js;
    js["msgid"] = REGIS;
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
    js["msgid"] = ONE_CHAT;
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
    js["msgid"] = ADD_FRI;
    js["friendid"] = toId;
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
    js["msgid"] = CREATE_GROUP;
    js["id"] = g_currentUser.getId();
    js["time"] = Timestamp::now().toFormattedString();
    js["groupname"] = groupName;
    js["groupdesc"] = groupDesc;
    send(js);
}

void joinGroup() {
    int groupid;
    std::cin >> groupid;
    std::cin.get();
    json js;
    js["msgid"] = JOIN_GROUP;
    js["groupid"] = groupid;
    js["id"] = g_currentUser.getId();
    send(js);
}

void chatGroup() {
    int groupid;
    std::cin >> groupid;
    std::cin.get();
    std::string msg;
    std::getline(std::cin, msg);
    json js;
    js["time"] = Timestamp::now().toFormattedString();
    js["msgid"] = GROUP_CHAT;
    js["groupid"] = groupid;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["msg"] = msg;
    send(js);
}

void showFriend() {
    json js;
    js["msgid"] = SHOW_FRI;
    js["id"] = g_currentUser.getId();
    send(js);
}

void showGroup() {
    json js;
    js["msgid"] = SHOW_GRO;
    js["id"] = g_currentUser.getId();
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
        memset(buf, 0, sizeof(buf));
        int len = ::recv(clientfd, buf, sizeof(buf), 0);
        if (len == -1) {
            std::cerr << "recv message failed\n";
            return ;
        }
        std::string msg = buf;
        json js = json::parse(msg);
        int msgid = js["msgid"];
        switch (msgid) {
            case LOGIN_ACK:
                logjs = js;
                cond.notify_one();
                break;
            case CHAT_ACK:
                chat(js);
                break;
            case SHOW_FRI_ACK:
                showfriend(js);
                break;
            case SHOW_GRO_ACK:
                showgroup(js);
                break;
            case REGIS_ACK:
                showregis(js);
                break;
            default:
                std::cout << "error msgid" << std::endl;
                break;
        }
    }
}


void showregis(json &js) {
    printf("hre\n");
    int id = js["id"];
    printf("Your id is %d\n", id);
}

void chat(json &js) {
    printf("%s:", js["time"].dump().c_str());
    if (js.contains("groupname"))
        printf("%s:", js["groupname:"].dump().c_str());
    printf("%s:%s\n", js["name"].dump().c_str(), js["msg"].dump().c_str());
    printf("-------------------\n");
}

void showfriend(json &js) {
    std::vector<std::string> friends = js["friend"];
    for (auto fri : friends) {
        json js = json::parse(fri);
        printf("%s:%s:%s\n", js["id"].dump().c_str(), js["name"].dump().c_str(), js["state"].dump().c_str());
    }
    printf("-------------------\n");
}

void showgroup(json &js) {
    std::vector<std::string> groups = js["group"];
    for (auto group : groups) {
        json js = json::parse(group);
        printf("%s:%s:%s\n", js["groupid"].dump().c_str(), js["groupname"].dump().c_str(), js["groupdesc"].dump().c_str());
    }
    printf("-------------------\n");
}