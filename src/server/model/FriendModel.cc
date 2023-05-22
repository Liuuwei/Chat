#include "FriendModel.h"

#include "Mysql.h"

bool FriendModel::insert(int userId, int friendId) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "insert into Friend values(%d, %d)", userId, friendId);
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return true;
        }
    }
    return false;
}

bool FriendModel::remove(int userId, int friendId) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "delete from Friend where userid = %d and friendid = %d", userId, friendId);
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return true;
        }
    }
    return false;
}

std::vector<User> FriendModel::query(int userId) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "select a.id,a.name,a.state from User a inner join Friend b on b.friendid = a.id where b.userid = %d", userId);

    std::vector<User> vec;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES *res = mysql.query(sql);
        if (res) {
            MYSQL_ROW row;
            while ( (row = mysql_fetch_row(res)) != nullptr) {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}