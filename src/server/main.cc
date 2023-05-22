#include "ChatServer.h"
#include "ChatService.h"
#include <iostream>

#include <signal.h>

void sigHandler(int sig) {
    ChatService::instance()->resetState();
    exit(0);
}

int main() {
    signal(SIGINT, sigHandler);
    EventLoop loop;
    InetAddress addr(9999);
    ChatServer server(&loop, addr, "ChatServer");
    server.start();
    loop.loop();
    return 0;
}