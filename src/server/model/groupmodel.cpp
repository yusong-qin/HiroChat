#include "groupmodel.hpp"
#include "db.h"

bool GroupModel::insert(User &user,Group &group){
    //组装sql语句
    char sql[1024] = {0};

    sprintf(sql,"insert into ALLGroup(groupname,groupdesc) values('%s','%s')",
            group.getName().c_str(),group.getDesc().c_str());
    
    MySQL mysql;
    if(mysql.connect()){
        if(mysql.update(sql)){
            
            //如果群组创建成功，将群组和创建群的用户绑定，加入到GroupUser表中
            group.setId(mysql_insert_id(mysql.getConnection()));
            char sql_1[1024] = {0};
            sprintf(sql_1,"insert into GroupUser values('%d','%d','%s')",
            group.getId(),user.getId(),"creator");
            MySQL mysql_2;
            if(mysql_2.connect()){
                mysql_2.update(sql_1);
                return true;
            }
        }
    }
    return false;
}

void GroupModel::addGroup(int userid,int groupid,string role){
    //组装sql语句
    char sql[1024] = {0};

    //insert
    sprintf(sql,"insert into GroupUser(groupid,userid) values('%d','%d')",
        groupid,userid);

    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);
    }
}

vector<Group> GroupModel::queryGroups(int id){
    //组装sql语句 多表联查
    char sql[1024] = {0};

    //先根据用户id在GroupUser表中查询出该用户所属的群组信息
    //再根据群组信息，查询属于该群组的所有用户的id，并且和user表进行多表联合查询，查出该用户的详细信息
    sprintf(sql,"select a.id,a.groupname,a.groupdesc from ALLGroup a inner join GroupUser b on a.id = b.groupid where b.userid = %d",id);
    
    vector<Group> groupVec;
    
    MySQL mysql;
    if(mysql.connect()){
        MYSQL_RES  *res = mysql.query(sql);
        if(res != nullptr){
            MYSQL_ROW row;
            //查出userid所有的群组信息
            while((row = mysql_fetch_row(res)) != nullptr){
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);
            }
            mysql_free_result(res);
        }
    }
    //查询群组的用户信息
    for(Group &group:groupVec){
        sprintf(sql,"select a.id,a.name,a.state,b.grouprole from User a inner join GroupUser b on b.userid = a.id where b.groupid = %d",
            group.getId());
        
        MYSQL_RES *res = mysql.query(sql);
        if(res!=nullptr){
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr)
            {
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

    return groupVec;

}


// 查询群组人员 用于给除自己之外的人发送消息
// 根据指定的groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其它成员群发消息
vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    char sql[1024] = {0};
    sprintf(sql, "select userid from GroupUser where groupid = %d and userid != %d", groupid, userid);

    vector<int> idVec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return idVec;
}