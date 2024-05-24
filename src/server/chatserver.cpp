#include "chatserver.hpp"
#include "chatservice.hpp"
#include "json.hpp"
#include <functional>
#include <string>
using namespace std;
using namespace placeholders;
using json = nlohmann::json;



//初始化聊天服务器对象
ChatServer::ChatServer(EventLoop *loop,
                const InetAddress &listenAddr,
                const string &nameAge)
                :_server(loop,listenAddr,nameAge)
                ,_loop(loop){
                    //注册连接回调
                    _server.setConnectionCallback(bind(&ChatServer::onConnection,this,_1));
                    //注册消息回调
                    _server.setMessageCallback(bind(&ChatServer::onMessage,this,_1,_2,_3));

                    //设置线程数量
                    _server.setThreadNum(4);

                }


//启动服务
void ChatServer::start(){
    _server.start();
}


// 上报连接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr& conn){
    //客户端断开连接
    if(!conn->connected()){
        // 客户端异常退出
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}

// 上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr& conn,
                            Buffer* buffer,
                            Timestamp time)
{
    //将客户端读来的数据解析成字符串
    string buf =  buffer->retrieveAllAsString();                        

    //数据的反序列化
    json js = json::parse(buf);   

    //达到的目的:完全解耦网络模块的代码和业务模块的代码
    // 通过js["msgid"] 获取 => 业务handler =>conn 或 js对象 或 time ...            
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());

    // 回调消息绑定好的时间处理器，来执行相应的业务处理
    msgHandler(conn,js,time);
}