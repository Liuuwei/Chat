#include "UserModel.h"

#include "Mysql.h"

bool UserModel::insert(User &user) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "insert into User(name, password, state) values('%s', '%s', '%s')", 
                  user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

User UserModel::query(int id) {
    User user;
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "select * from User where id = %d", id);
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES *res;
        if ( (res = mysql.query(sql))) {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row) {
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
            }
        }
    }
    return user;
}

bool UserModel::updateState(const User &user) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "update User set state = '%s' where id = %d", user.getState().c_str(), user.getId());
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return true;
        }
    }
    return false;
}

bool UserModel::resetState() {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "update User set state = 'offline' where state = 'online'");
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql))
            return true;
    }
    return false;
}