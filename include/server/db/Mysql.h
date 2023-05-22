#pragma

#include <mysql/mysql.h>

#include <string>

class MySQL {
    public:
        MySQL();
        ~MySQL();
        bool connect();
        bool update(const std::string &sql);
        MYSQL_RES* query(const std::string &sql);
        MYSQL* getConnection() const;
    private:
        MYSQL *conn_;
};