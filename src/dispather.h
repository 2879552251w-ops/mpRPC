#include <google/protobuf/message.h>
#include <muduo/net/TcpServer.h>
#include <muduo/base/noncopyable.h>
#include <map>
#include <type_traits>

typedef std::shared_ptr<google::protobuf::Message> MessagePtr;

class Callback : muduo::noncopyable
{
public:
    virtual ~Callback()=0;
    virtual void onMessage(const muduo::net::TcpConnectionPtr& conn,
                        const MessagePtr& mess,
                        muduo::Timestamp) =0;
};

template<class T>
class CallbackT : public Callback
{
    static_assert(std::is_base_of<google::protobuf::Message,T>::value,
                    "T must be derived from Message");

public:
typedef std::function<void(const muduo::net::TcpConnectionPtr&,
                        const std::shared_ptr<T>&,
                        muduo::Timestamp)>  ProtobufCallbackT;

    Callback(const ProtobufCallbackT& call)
    :callback_(call)
    {

    }

    void onMessage(const muduo::net::TcpConnectionPtr& conn,
                        const MessagePtr& mess,
                        muduo::Timestamp receivetime) const override
    {
        callback_(conn,mess,receivetime);
    }

private:
    ProtobufCallbackT callback_;
};

class ProtobufDispather
{
public:
typedef std::function<void(const muduo::net::TcpConnectionPtr&,
                        const MessagePtr&,
                        muduo::Timestamp)>  ProtobufCallback;

ProtobufDispather(const ProtobufCallback& defaultcallback):defaultcallback_(defaultcallback)
{
}

void onProtobufMessage(const muduo::net::TcpConnectionPtr& conn,
                        const MessagePtr& mess,
                        muduo::Timestamp receivetime)
{
    CallbackMap::const_iterator it=callbacks_.find(mess->GetDescriptor());
    if(it!=callbacks_.end())
    {
        it->second->onMessage(conn,mess,receivetime);
    } 
    else
    {
        defaultcallback_(conn,mess,receivetime);
    }
}

template<typename T>
void registerMessageCallback(const typename CallbackT<T>::ProtobufCallbackT& callback)
{
    std::shared_ptr<CallbackT<T>> sp(new CallbackT<T>(callback));
    callbacks_[T::descriptor()] = sp;
}

private:
 typedef  std::map<const google::protobuf::Descriptor*,std::shared_ptr<Callback> > CallbackMap;
 
 CallbackMap callbacks_;
 ProtobufCallback defaultcallback_;
};