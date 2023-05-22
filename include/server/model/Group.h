#pragma once

#include "GroupUser.h"

#include <string>
#include <vector>

class Group {
    public:
        Group(int id = -1, std::string name = "", std::string desc = "") :
            id_(id), name_(name), desc_(desc) {
            
        }

        void setId(int id) { id_ = id;}
        void setName(const std::string &name) { name_ = name; }
        void setDesc(const std::string &desc) { desc_ = desc; }
        int getId() const { return id_; }
        std::string getName() const { return name_; }
        std::string getDesc() const { return desc_; }
        std::vector<GroupUser>& getUsers() { return users_; }
    private:
        int id_;
        std::string name_;
        std::string desc_;
        std::vector<GroupUser> users_;
};