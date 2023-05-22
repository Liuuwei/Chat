#pragma once

#include "User.h"

#include <vector>

class FriendModel {
    public:
        bool insert(int userId, int friendId);
        bool remove(int userId, int friendId);
        std::vector<User> query(int userId);
    private:
};