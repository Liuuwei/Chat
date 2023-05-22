#pragma once

#include <string>

class User {
    public:
        User(std::string name = "", std::string pwd = "") : 
            id_(-1), name_(name), password_(pwd), state_("offline") {
            
        }
        void setId(int id) { id_ = id; }
        void setName(std::string name) { name_ = name; }
        void setPassword(std::string password) { password_ = password; }
        void setState(std::string state) { state_ = state; }

        int getId() const { return id_; }
        std::string getName() const { return name_; }
        std::string getPassword() const { return password_; }
        std::string getState() const { return state_; }
    protected:
        int id_;
        std::string name_;
        std::string password_;
        std::string state_;
};