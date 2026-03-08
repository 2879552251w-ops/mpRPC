#pragma once
#include "google/protobuf/service.h"
#include <muduo/net/TcpConnection.h>
#include <memory>

class myChannel :public google::protobuf::RpcChannel
{
public:
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done);
    void setConnection(muduo::net::TcpConnectionPtr conn){conn_=conn;}
private:
    muduo::net::TcpConnectionPtr conn_;
};