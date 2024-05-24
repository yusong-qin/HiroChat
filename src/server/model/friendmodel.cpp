#include "friendmodel.hpp"
#include "db.h"

// 添加好友关系
void FriendModel::insert(int userid,int friendid){
    //组装sql语句
    char sql[1024] = {0};
    char sql1[1024] = {0};
    sprintf(sql,"insert into Friend values(%d,%d)",userid,friendid);
    sprintf(sql,"insert into Friend values(%d,%d)",friendid,userid);

    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);
        mysql.update(sql1);
    }

    
}

//返回用户好友列表 该表中有好友id 好友name 好友在线状态  —— User表和Friend表联合查询
vector<User> FriendModel::query(int userid){

    char sql[1024] = {0};
    sprintf(sql,"select a.id,a.name,a.state from User a inner join Friend b on b.friendid = a.id where b.userid = %d",userid);

    vector<User> vec;
    MySQL mysql;
    if(mysql.connect()){
        MYSQL_RES *res =  mysql.query(sql);
        if(res != nullptr){
            // 把userid用户所有的离线消息放入vec中返回
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr){
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