#pragma once
#include <rpcclient.h>
#include "google/protobuf/service.h"
#include <muduo/net/TcpConnection.h>
#include <memory>

class myChannel :public google::protobuf::RpcChannel
{
public:
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done);
private:
};

class longChannel :public google::protobuf::RpcChannel
{
public:
    longChannel(muduo::net::EventLoop* loop,muduo::net::InetAddress addr)
    :rpcclient_(new dongxia::RpcClient(loop,addr))
    {
    }
    virtual void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done);

private:
    using rpcClientPtr = std::shared_ptr<dongxia::RpcClient>;
    rpcClientPtr rpcclient_;
};
