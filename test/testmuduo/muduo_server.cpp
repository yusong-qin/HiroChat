/*
muduo网络库给用户提供了两个主要的类
TcpServer:用于编写服务器程序
TcpClient:用于编写客户端程序

epoll + 线程池
好处：能把网络I/O代码和业务代码分开
						用户的连接和断开 用户的可读写事件
*/


#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
#include <string>

using namespace muduo;
using namespace muduo::net;
using namespace std;
/*
基于muduo网络库开发服务器程序 ：
1.组合TcpServer对象

2.创建EventLoop事件循环对象的指针

3.明确TcpServer构造函数需要什么参数，输出ChatServer的构造函数

4.在当前服务器类的构造当中，注册处理连接的回调函数和处理读写事件的回调函数

5.设置合适的服务端线程数量，muduo库会自动划分IO线程和工作线程
*/

class ChatServer {
public:
	ChatServer(EventLoop* loop,				//事件循环——反应堆
		const InetAddress& listenAddr,		//IP  +Port
		const string& nameArg)				//服务器名字
		: _server(loop, listenAddr, nameArg)
		, _loop(loop)
	{
		//回调：一般的函数调用知道什么时候发生，以及发生之后要做什么事情
		//而譬如用户注册事件，不知道什么时候发生，这时候就需要muduo网络库进行监听，并在该事件发生时上报给改函数 
		// 即： 函数发生的时间和发生之后要做什么不在同一时刻

		//给服务器注册  用户连接的创建和断开   回调(调用onConnection方法)
		_server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, placeholders::_1));

		//给服务器注册用户读写事件回调
		_server.setMessageCallback(std::bind(&ChatServer::onMessage, this, placeholders::_1, placeholders::_2, placeholders::_3));

		//设置服务器端的线程数量
		_server.setThreadNum(4); //1个IO线程,3个工作线程
	}
	//开启事件循环
	void start() {
		_server.start();
	}

private:
	//专门处理用户的连接创建和断开 相当于epoll中发现是监听文件描述符listenfd，执行accept的过程，该函数只暴露了连接的接口
	void onConnection(const TcpConnectionPtr& conn) {

		if (conn->connected()) {
			cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << "state:online" << endl;
		}
		else {
			cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << "state:offline" << endl;
			conn->shutdown();  //close(fd)
			// _loop->quit(); // 相当于断开epoll
		}
	}

	//专门处理用户的读写事件
	void onMessage(const TcpConnectionPtr& conn,// 连接
		Buffer* buffer, // 缓冲区
		Timestamp time) { //接收到数据的时间信息
		string buf = buffer->retrieveAllAsString();
		cout << "recv data:" << buf << "time:" << time.toString() << endl;
		conn->send(buf);
	}

	TcpServer _server;  // #1
	EventLoop* _loop;	// #2 epoll
};

int main() {
	EventLoop loop; //epoll
	InetAddress addr("192.168.18.128",6000);

	ChatServer server(&loop,addr,"ChatServer"); //通过muduo库创建起了server对象
	server.start();//相当于把listenfd 以 epoll_ctl() 添加到 epoll上
	loop.loop();//相当于epoll_wait 以阻塞方式等待：新用户连接、已连接用户的读写事件等

	return 0;
}