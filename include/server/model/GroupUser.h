#pragma once

#include "User.h"

class GroupUser : public User{
    public:
        void setRole(const std::string &role) { role_ = role; }
        std::string getRole() const { return role_; }
    private:
        std::string role_;
};