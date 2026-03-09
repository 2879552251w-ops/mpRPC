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
    :client_(new dongxia::RpcClient(loop,addr))
    {
    }
    virtual void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done);

private:
    using rpcClientPtr = std::shared_ptr<dongxia::RpcClient>;
    rpcClientPtr client_;
};

void longChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                             google::protobuf::RpcController *controller,
                             const google::protobuf::Message *request,
                             google::protobuf::Message *response,
                             google::protobuf::Closure *done)
{
    RpcMessage message;
    message.set_type(REQUEST);
    int64_t id = id_.incrementAndGet();
    message.set_id(id);
    message.set_service(method->service()->full_name());
    message.set_method(method->name());
    message.set_request(request->SerializeAsString()); // FIXME: error check

    OutstandingCall out = {response, done};
    {
        MutexLockGuard lock(mutex_);
        outstandings_[id] = out;
    }
    codec_.send(conn_, message);
}