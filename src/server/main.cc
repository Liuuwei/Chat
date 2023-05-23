#include "ChatServer.h"
#include "ChatService.h"
#include <iostream>

#include <signal.h>

void sigHandler(int sig) {
    ChatService::instance()->resetState();
    ChatService::instance()->resetRedis();
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, sigHandler);
    EventLoop loop;
    InetAddress addr(atoi(argv[1]));
    ChatServer server(&loop, addr, "ChatServer");
    server.start();
    loop.loop();
    return 0;
}