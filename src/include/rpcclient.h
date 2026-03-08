#pragma once

#include <dispatcher.h>
#include <codec.h>
#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>
#include <muduo/net/Callbacks.h>

class RpcMessage;
using RpcMessagePtr = std::shared_ptr<RpcMessage>;

class RpcClient
{
public:
    RpcClient(muduo::net::EventLoop *loop, const muduo::net::InetAddress &serveraddr)
        : loop_(loop),
          client_(loop_, serveraddr, "rpcclient"),
          dispatcher_(std::bind(&RpcClient::onUnknownMessage, this, muduo::_1, std::placeholders::_2, std::placeholders::_3)),
          codec_(std::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_, muduo::_1, muduo::_2, muduo::_3))
    {
        dispatcher_.registerMessageCallback<RpcMessage>(std::bind(&RpcClient::onRpcMessage, this, muduo::_1, muduo::_2, muduo::_3));
        client_.setMessageCallback(std::bind(&RpcClient::onmessage, this, muduo::_1, muduo::_2, muduo::_3));
        client_.setConnectionCallback(std::bind(&RpcClient::onConnection, this, muduo::_1));
    }

    void connect()
    {
        client_.connect();
    }

    void onUnknownMessage(const muduo::net::TcpConnectionPtr &,
                          const MessagePtr &message,
                          muduo::Timestamp)
    {
        LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
    }
    void onmessage(const muduo::net::TcpConnectionPtr &conn,
                   muduo::net::Buffer *buf,
                   muduo::Timestamp recvtime)
    {
        codec_.onMessage(conn, buf, recvtime);
    }

    void onConnection(const muduo::net::TcpConnectionPtr &conn);

    void onRpcMessage(const muduo::net::TcpConnectionPtr &conn,
                      const RpcMessagePtr &messagePtr,
                      muduo::Timestamp resvtime);

private:
    muduo::net::EventLoop *loop_;
    muduo::net::TcpClient client_;
    ProtobufDispatcher dispatcher_;
    ProtobufCodec codec_;
    struct OutstandingCall
    {
        ::google::protobuf::Message *response;
        ::google::protobuf::Closure *done;
    };
    using callbackMap = std::map<int64_t, OutstandingCall>;
    std::mutex mutex_;
    callbackMap outstandings_;
};