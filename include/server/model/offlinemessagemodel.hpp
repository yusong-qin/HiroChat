#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

#include "db.h"
#include <string>
#include <vector>

using namespace std;
//提供离线消息表的操作接口方法
class OfflineMsgModel{
public:
    //存储用户的离线消息
    void insert(int userid,string msg);

    //删除用户的离线消息
    void remove(int userid);

    //用户上线后检查离线消息列表，有消息弹出
    //查询该用户所有的离线消息
    vector<string> query(int userid);
private:

};

#endif