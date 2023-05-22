#pragma once

#include "User.h"

#include <memory>

class UserModel {
    public:
        bool insert(User &user);
        User query(int id);
        bool updateState(const User &user);
        bool resetState();
    private:
};