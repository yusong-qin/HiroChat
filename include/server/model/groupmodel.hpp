#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "user.hpp"
#include "group.hpp"
#include "groupuser.hpp"
#include <vector>

using namespace std;

// 维护群组信息的操作接口方法
class GroupModel{
public:
    // 创建群组
    bool insert(User &user,Group &group);

    // 加入群聊
    void addGroup(int userid,int groupid,string role);

    // 查询用户所在群组信息
    vector<Group> queryGroups(int id);

    // 查询群组人员 用于给除自己之外的人发送消息
    vector<int> queryGroupUsers(int id,int groupid);

};

#endif