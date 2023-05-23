#include "GroupModel.h"

#include "Mysql.h"

bool GroupModel::createGroup(Group &group) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "insert into AllGroup (groupname,groupdesc) values('%s','%s')", group.getName().c_str(), group.getDesc().c_str());
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

bool GroupModel::joinGroup(int userId, int groupId, std::string role) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "insert into GroupUser (groupId,userId,grouprole) values(%d,%d,'%s')", groupId, userId, role.c_str());
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            return true;
        }
    }
    return false;
}

// * 加入的所有组，以及组中其他成员的状态
std::vector<Group> GroupModel::queryGroups(int userId) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "select a.id,a.groupname,a.groupdesc from AllGroup a inner join GroupUser b on a.id = b.groupid where b.userid = %d", userId);
    MySQL mysql;
    std::vector<Group> GroupVec;
    
    if (mysql.connect()) {
        MYSQL_RES *res;
        res = mysql.query(sql);
        if (res) {
            MYSQL_ROW row;
            while ( (row = mysql_fetch_row(res)) != nullptr) {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                GroupVec.push_back(group);
            }
            mysql_free_result(res);

            for (auto group : GroupVec) {
                snprintf(sql, sizeof(sql), "select a.id,a.name,a.state,b.grouprole from User a inner join GroupUser b on b.userid = a.id where b.groupid = %d", group.getId());
                res = mysql.query(sql);
                if (res) {
                    while ( (row = mysql_fetch_row(res)) != nullptr) {
                        GroupUser user;
                        user.setId(atoi(row[0]));
                        user.setName(row[1]);
                        user.setState(row[2]);
                        user.setRole(row[3]);
                        group.getUsers().push_back(user);
                    }
                    mysql_free_result(res);
                }
            }
        }
    }

    return GroupVec;
}

// * 查询群组中除了自己以外的其他所有成员
std::vector<int> GroupModel::queryGroupUsers(int userId, int groupId) {
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), "select userid from GroupUser where groupid = %d and userid <> %d", groupId, userId);
    MySQL mysql;
    std::vector<int> userVec;
    
    if (mysql.connect()) {
        MYSQL_RES *res;
        res = mysql.query(sql);
        if (res) {
            MYSQL_ROW row;
            while ( (row = mysql_fetch_row(res))) {
                userVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return userVec;
}