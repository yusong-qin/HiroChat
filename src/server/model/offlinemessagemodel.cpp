#include "offlinemessagemodel.hpp"


void OfflineMsgModel::insert(int userid,string msg){
    //1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql,"insert into OfflineMessage(userid,message) values('%d','%s')",
        userid,msg.c_str());

    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);
    }

}

//删除用户的离线消息
void OfflineMsgModel::remove(int userid){
    char sql[1024] = {0};
    sprintf(sql,"delete from OfflineMessage where userid=%d",userid);
    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);
    }
}

//用户上线后检查离线消息列表，有消息弹出
//查询该用户所有的离线消息
vector<string> OfflineMsgModel::query(int userid){
//1. 组装sql语句
    char sql[1024] = {0};
    sprintf(sql,"select message from OfflineMessage where userid = %d",userid);

    vector<string> vec;
    MySQL mysql;
    if(mysql.connect()){
        MYSQL_RES *res =  mysql.query(sql);
        if(res != nullptr){
            // 把userid用户所有的离线消息放入vec中返回
            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr){
                vec.push_back(row[0]);
            }

            mysql_free_result(res);
        } 
    }
    return vec;
}