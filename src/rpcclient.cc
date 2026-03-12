#include <rpcclient.h>
#include <rpcproto.pb.h>
#include <assert.h>
using namespace dongxia;
void RpcClient::onConnection(const muduo::net::TcpConnectionPtr &conn)
    {
        LOG_INFO << conn->localAddress().toIpPort() << " -> "
                 << conn->peerAddress().toIpPort() << " is "
                 << (conn->connected() ? "UP" : "DOWN");

        if (conn->connected())
        {
            LOG_ERROR << "who?";
        }
        else
        {
            loop_->quit();
        }
    }

void RpcClient::onRpcMessage(const muduo::net::TcpConnectionPtr &conn,
                      const RpcMessagePtr &messagePtr,
                      muduo::Timestamp resvtime)
    {
        RpcMessage &message = *messagePtr;
        if (message.type() == dongxia::RESPONSE)
        {
            int64_t id = message.id();
            assert(message.has_response() || message.has_error());

            OutstandingCall out = {NULL, NULL};

            {
                std::lock_guard<std::mutex> lock(mutex_);
                std::map<int64_t, OutstandingCall>::iterator it = outstandings_.find(id);
                if (it != outstandings_.end())
                {
                    out = it->second;
                    outstandings_.erase(it);
                }
            }

            if (out.response)
            {
                std::unique_ptr<google::protobuf::Message> d(out.response);
                if (message.has_response())
                {
                    out.response->ParseFromString(message.response());
                }
                if (out.done)
                {
                    out.done->Run();
                }
            }
        }
    }


void RpcClient::insertClosure(int64_t id,const OutstandingCall& closure)
{
    insertclosureinloop(id,closure);
}

void RpcClient::insertclosureinloop(int64_t id,const OutstandingCall& closure)
{
    client_.connection()->getLoop()->runInLoop([this,closure,id]
    {
        this->outstandings_[id]=closure;
    });
}

void RpcClient::send(const google::protobuf::Message& msg)
{
    codec_.send(client_.connection(),msg);
}