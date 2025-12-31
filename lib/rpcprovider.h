#pragma once
#include <memory>
#include <unordered_map>
#include "google/protobuf/service.h"
#include "google/protobuf/descriptor.h"
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/base/Timestamp.h>

//框架提供宝贵的专门服务发布rpc服务的网络对象类
class RpcProvider
{
public:
    //这是框架提供给外部使用的，可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service *service);

    //启动rpc服务节点，开始提供rpc远程调用服务
    void Run();
private:
    std::unique_ptr<muduo::net::TcpServer> tcpserverPtr_;

    muduo::net::EventLoop loop_;
    //service服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service *service_;    //保存服务对象指针
        std::unordered_map<std::string,const google::protobuf::MethodDescriptor *> methodMap_;//保存服务方法的描述
    };
    //存储注册的服务信息,一个rpc服务可能有多个方法，比如UserService有Login，Reg，
    //一个服务端应该有多个rpc服务，比如FriendService，GroupService
    std::unordered_map<std::string,ServiceInfo> serviceMap_;

    void SendRpcResponse(const muduo::net::TcpConnectionPtr&,google::protobuf::Message* );
};