#include "Mysql.h"

#include <muduo/base/Logging.h>

static std::string server = "127.0.0.1";
static std::string user = "root";
static std::string password = "qmzh5233";
static std::string dbname = "chat";

MySQL::MySQL() {
    conn_ = mysql_init(nullptr);
}

MySQL::~MySQL() {
    if (conn_)
        mysql_close(conn_);
}

bool MySQL::connect() {
    MYSQL *p = mysql_real_connect(conn_, server.c_str(), user.c_str(), password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p)
        mysql_query(conn_, "set name gbk");
    
    return p;
}

bool MySQL::update(const std::string &sql) {
    if (mysql_query(conn_, sql.c_str()) != 0) {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "更新失败！";
        return false;
    }
    return true;
}

MYSQL_RES* MySQL::query(const std::string &sql) {
    if (mysql_query(conn_, sql.c_str()) != 0) {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":" << sql << "查询失败！";
        return nullptr;
    }
    return mysql_use_result(conn_);
}

MYSQL* MySQL::getConnection() const {
    return conn_;   
}