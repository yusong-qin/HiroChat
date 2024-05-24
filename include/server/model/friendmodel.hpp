#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include "user.hpp"
#include <vector>

using namespace std;

// 维护好友信息的操作接口方法
class FriendModel{
public:
    // 添加好友关系
    void insert(int userid,int friendid);

    //返回用户好友列表 该表中有好友id 好友name 好友在线状态  —— User表和Friend表联合查询
    vector<User> query(int userid);
};

#endif