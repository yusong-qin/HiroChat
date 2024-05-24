#ifndef PUBLIC_H
#define PUBLIC_H
/*
server和client的公共文件
*/

enum EnMsgType{
    LOGIN_MSG = 1,   //登录消息
    LOGIN_MSG_ACK,   //2登录响应消息
    LOGINOUT_MSG,    //注销消息
    REG_MSG,         //3注册消息
    REG_MSG_ACK,     //4注册响应消息
    ONE_CHAT_MSG,    //5聊天消息
    ADD_FRIEND,      //6添加好友消息

    CREATE_GROUP,    //7创建群组消息
    CREATE_GROUP_ACK,//8创建群组响应消息
    ADD_GROUP,       //9加入群组消息
    GROUP_CHAT_MSG,  //10群组聊天消息
};

#endif