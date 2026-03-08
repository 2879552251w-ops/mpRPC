#pragma once
#include "codec.h"
#include "dispatcher.h"

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

//message->codec->dispatcher->Rpc or other

class ProtoServer
{
    ProtoServer(muduo::net::EventLoop* loop,
        const InetAddress& listenAddr)
  : server_(loop, listenAddr, "ProtoServer"),
    dispatcher_([](const muduo::net::TcpConnectionPtr& conn,
                        const MessagePtr& mess,
                        muduo::Timestamp receivetime){conn->shutdown();}),
    codec_(std::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_, _1, _2, _3))
  {
    server_.setConnectionCallback(
        std::bind(&ProtoServer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&ProtobufCodec::onMessage, &codec_, _1, _2, _3));
  }

  void start()
  {
    server_.start();
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
        << conn->peerAddress().toIpPort() << " is "
        << (conn->connected() ? "UP" : "DOWN");
  }

  void onUnknownMessage(const TcpConnectionPtr& conn,
                        const MessagePtr& message,
                        Timestamp)
  {
    LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
    conn->shutdown();
  }


  TcpServer server_;
  ProtobufDispatcher dispatcher_;
  ProtobufCodec codec_;
};

