#pragma

#include <string>
#include <vector>

class OfflineMessageModel {
    public:
        bool insert(int userId, const std::string &msg);
        bool remove(int userId);
        std::vector<std::string> query(int userId);
    private:
};