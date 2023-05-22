#include "OfflineMessageModel.h"

#include "Mysql.h"

bool OfflineMessageModel::insert(int userId, const std::string &msg) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "insert into OfflineMessage values(%d, '%s')", userId, msg.c_str());
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return true;
        }
    }
    return false;
}

bool OfflineMessageModel::remove(int userId) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "delete from OfflineMessage where userid = %d", userId);
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> OfflineMessageModel::query(int userId) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "select message from OfflineMessage where userid = %d", userId);

    std::vector<std::string> vec;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES *res = mysql.query(sql);
        if (res) {
            MYSQL_ROW row;
            while ( (row = mysql_fetch_row(res)) != nullptr) {
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
        }
    }
    return vec;
}