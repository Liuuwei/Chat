#include "json.hpp"
#include "Public.h"
#include "Group.h"
#include "User.h"

#include <assert.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <iostream>
#include <vector>

using json = nlohmann::json;

// * 当前登录用户
User g_currentUser;
// * 好友列表
std::vector<User> g_currentUserFriendList;
// * 群组列表
std::vector<Group> g_currentUserGroupList;
// * 显示用户信息
void showCurrentUserData();

// * 接收线程
void readTaskHandler(int clientfd);
// * 获取系统事件
std::string getCurrentTime();

void mainMenu();
void funMenu();

bool login();
void regis();

int clientfd;

int main(int argc, char *argv[]) {
    if (argc < 3) {
        exit(-1);
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
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
    if (len == -1) {
        std::cerr << "send login message error\n";
        return false;
    }
    return true;
}

void funMenu() {
    char buf[1024] = {0};
    int len = ::recv(clientfd, buf, sizeof(buf), 0);
    std::string msg = buf;
    json js = json::parse(msg);
    if (js["errno"] != 0) {
        printf("%s\n", js["errmsg"].dump().c_str());
        return ;
    } 
    g_currentUser.setId(js["id"].get<int>());
    g_currentUser.setName(js["name"].dump());

    if (js.contains("friend")) {
        std::vector<std::string> friends = js["friend"];
        for (auto friend : friends) {
            User user;
            json frijs = json::parse(friend);
            user.setId(frijs["id"].get<int>());
            user.setName(frijs["name"]);
            user.setState(frijs["state"]);
            g_currentUserFriendList.push_back(user);
        }
    }

    if (js.contains("group")) {
        std::vector<std::string> groups = js["group"];
        for (auto group : groups)
    }
}