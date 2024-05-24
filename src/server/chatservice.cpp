#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <vector>

using namespace std;
using namespace muduo;

//获取单例对象的接口函数
ChatService* ChatService::instance(){
    static ChatService service;
    return &service;
}

//注册消息以及对应Handler回调操作
ChatService::ChatService(){
    _msgHandlerMap.insert({LOGIN_MSG,std::bind(&ChatService::login,this,_1,_2,_3)});
    _msgHandlerMap.insert({REG_MSG,std::bind(&ChatService::reg,this,_1,_2,_3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG,std::bind(&ChatService::oneChat,this,_1,_2,_3)});
    _msgHandlerMap.insert({ADD_FRIEND,std::bind(&ChatService::addFriend,this,_1,_2,_3)});
    _msgHandlerMap.insert({CREATE_GROUP,std::bind(&ChatService::creatGroup,this,_1,_2,_3)});
    _msgHandlerMap.insert({ADD_GROUP,std::bind(&ChatService::addGroup,this,_1,_2,_3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG,std::bind(&ChatService::groupChat,this,_1,_2,_3)});
    _msgHandlerMap.insert({LOGINOUT_MSG,std::bind(&ChatService::loginout,this,_1,_2,_3)});
}

//处理登录业务 
void ChatService::login(const TcpConnectionPtr& conn,json &js,Timestamp time){
    int id = js["id"].get<int>();
    string pwd = js["password"];

    User user = _userModel.query(id);
    if(user.getId() == id && user.getPwd()==pwd){
        if(user.getState()=="online"){
            // 该用户已登录，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["error"] = 1;
            response["errmsg"] = "该账号已登录，请重新输入新账号";
            conn->send(response.dump());
        }
        else{
            //登陆成功，记录用户连接信息
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id,conn});
            }
            
            // 登陆成功，更新用户状态信息 state offline=>online
            user.setState("online");
            _userModel.updateState(user);  //更新user表状态

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["error"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            
            //查询该用户是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if(!vec.empty()){
                response["offlienmsg"] = vec;
                _offlineMsgModel.remove(id);
            } 
        
            //查询该用户的好友信息并返回
            vector<User> userVec = _friendModel.query(id);
            if(!userVec.empty()){
                vector<string> vec2;
                for(User &user:userVec){
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }

            //查询该用户的群组信息并返回
            vector<Group> groupuserVec = _groupModel.queryGroups(id);
            if (!groupuserVec.empty())
            {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string> groupV;
                for (Group &group : groupuserVec)
                {
                    json grpjson;
                    grpjson["id"] = group.getId();
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupV.push_back(grpjson.dump());
                }

                response["groups"] = groupV;
            }
            conn->send(response.dump());
        }
        
    }
    else{
        // 该用户不存在,登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["error"] = 1;
        response["errmsg"] = "用户名或密码错误";
        conn->send(response.dump());
        
    }
    
}
//处理注册业务
void ChatService::reg(const TcpConnectionPtr& conn,json &js,Timestamp time){
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);

    bool state =  _userModel.insert(user);
    if(state){
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["error"] = 0;
        response["id"] = user.getId();

        conn->send(response.dump());

    }
    else{
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["error"] = 1;

        conn->send(response.dump());
    }
}


//服务器异常,业务重置方法
void ChatService::reset(){
    // 把online状态 的用户设置成offline
    _userModel.resetState();

}

// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid){
    //记录错误日志，msgid没有对应的事件处理回调
    auto it = _msgHandlerMap.find(msgid);
    if(it == _msgHandlerMap.end()){
        
        //返回一个默认的处理器，空操作
        return [=](const TcpConnectionPtr& conn,json &js,Timestamp time){
                LOG_ERROR<<"msgid:"<<msgid<<"can not find handler";
            };
    }
    else{
        return _msgHandlerMap[msgid];
    }
   
}

//处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn){
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for(auto it =_userConnMap.begin();it!=_userConnMap.end();++it){
            if(it->second == conn){
                user.setId(it->first);
                // 从map表删除用户的连接信息
                _userConnMap.erase(it);
                break;
            }
        }
    }
    // 更新的用户的状态信息
    if(user.getId()!=-1){
        user.setState("offline");
        _userModel.updateState(user);
    }
}

// 登出业务
void ChatService::loginout(const TcpConnectionPtr& conn,json &js,Timestamp time){
    int userid = js["id"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if(it!=_userConnMap.end()){
            _userConnMap.erase(it);
        }
    }
    // 更新的用户的状态信息
    User user(userid,"","","offline");
    user.setState("offline");
    _userModel.updateState(user);
}

// 一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr& conn,json &js,Timestamp time){
    int toid = js["toid"].get<int>();

    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(toid);
    if(it != _userConnMap.end()){
        //toid在线，转发消息
        it->second->send(js.dump());
        return;
    }
    else{
    //toid不在线，存储离线消息
        js["msgid"] = ONE_CHAT_MSG;
        _offlineMsgModel.insert(toid,js.dump());

    }
}

void ChatService::addFriend(const TcpConnectionPtr& conn,json &js,Timestamp time){
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    //存储好友信息
    _friendModel.insert(userid,friendid);
}

void ChatService::creatGroup(const TcpConnectionPtr& conn,json &js,Timestamp time){
    User user;
    user.setId(js["id"].get<int>());

    Group group;
    group.setName(js["groupname"]);
    if(js.find<string>("groupdesc")!=js.end()){
        group.setDesc(js["groupdesc"]);
    }
    
    //存储群组信息
    bool isCreat = _groupModel.insert(user,group);
    if(isCreat){
        // 注册群组成功
        json response;
        response["msgid"] = CREATE_GROUP_ACK;
        response["error"] = 0;
        response["groupid"] = group.getId();

        conn->send(response.dump());
    }
    else{
        json response;
        response["msgid"] = CREATE_GROUP_ACK;
        response["error"] = 1;
        conn->send(response.dump());
    }
}

// 加入群组
void ChatService::addGroup(const TcpConnectionPtr& conn,json &js,Timestamp time){
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    string role = "normal";
    if(js.contains("role")){
        string role = js["role"];
    }
    else{
        string role = "normal";
    }
    
    _groupModel.addGroup(userid,groupid,role);
}

// 群组聊天
void ChatService::groupChat(const TcpConnectionPtr& conn,json &js,Timestamp time){
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid,groupid);
    lock_guard<mutex> lock(_connMutex);
    for(int id : useridVec){
        auto it = _userConnMap.find(id);
        if(it != _userConnMap.end()){
            //转发群消息
            it->second->send(js.dump());
        }
        else{
            //存储离线群消息
            js["msgid"] = GROUP_CHAT_MSG;
            _offlineMsgModel.insert(id,js.dump());
        }
    }
}