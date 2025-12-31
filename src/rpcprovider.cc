#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "zookeeperutil.h"

// 这是框架提供给外部使用的，可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;
    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务名字
    std::string service_name = pserviceDesc->name();
    // 获取服务对象的方法的数量
    int methodCnt = pserviceDesc->method_count();

    for (int i = 0; i < methodCnt; i++)
    {
        // 存储方法的名字和描述映射
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        service_info.methodMap_[pmethodDesc->name()] = pmethodDesc;
    }
    // 存储方法的对象指针
    service_info.service_ = service;
    serviceMap_[service_name] = service_info;
}

// 启动rpc服务节点，开始提供rpc远程调用服务
void RpcProvider::Run()
{
    const std::string &ip = MprpcApplication::getconfig().Load("rpcserverip");
    uint16_t port = std::stoi(MprpcApplication::getconfig().Load("rpcserverport"));
    muduo::net::InetAddress address(ip, port);
    // 创建tcpserver
    muduo::net::TcpServer server(&loop_, address, "RpcProvider");
    // 绑定回调
    server.setConnectionCallback([](const muduo::net::TcpConnectionPtr &conn)
                                 {
        if(!conn->connected())
        {
            conn->shutdown();
        } });
    // 要携带服务名字，方法名字，参数,
    // message格式设定为 heardsize + header + body ，
    // 其中header和body都是protobuf中自定义类型的序列化
    server.setMessageCallback([this](const muduo::net::TcpConnectionPtr &conn,
                                     muduo::net::Buffer *msg,
                                     muduo::Timestamp)
                              {
             // 网络编程核心：解决 TCP 粘包问题
    // 循环读取，因为一次数据到达可能包含多个完整的 RPC 请求
    while (msg->readableBytes() >= 4) 
    {
        // 1. 先 peek (只读不走) 前4个字节，获取 header_size
        //    peek() 返回的是缓冲区可读数据的首地址，不会移动 readerIndex
        const char* data = msg->peek();
        uint32_t header_size = 0;
        // 从 buffer 中读取4字节到 header_size，注意字节序（这里假设本地字节序）
        header_size = *reinterpret_cast<const uint32_t*>(data);
        header_size=ntohl(header_size);  //防止主机字节序不一致，对方要调用htonl
        
        // 2. 判断缓冲区剩余数据是否足够读取 Header
        //    如果不够，说明包不完整，跳出循环等待下一次数据到达
        if (msg->readableBytes() < 4 + header_size) {
            break;
        }

        // 3. 读取并反序列化 Header
        //    通过指针偏移直接构造 string，避免移动 buffer 指针
        std::string rpc_header_str(data + 4, header_size);
        rpcheader::RequreMsg rpcHeader;
        std::string service_name;
        std::string method_name;
        uint32_t args_size;

        if (rpcHeader.ParseFromString(rpc_header_str))
        {
            service_name = rpcHeader.service_name();
            method_name = rpcHeader.method_name();
            args_size = rpcHeader.args_size();
        }
        else
        {
            // Header 反序列化失败，这通常意味着数据严重损坏
            // 此时不仅要 break，可能还需要关闭连接或丢弃这段缓冲区
            std::cout << "rpc_header_str:" << rpc_header_str << " parse error!" << std::endl;
            //直接关闭连接算了
            conn->shutdown();
            break; 
        }

        // 4. 判断缓冲区剩余数据是否足够读取 Args (Body)
        if (msg->readableBytes() < 4 + header_size + args_size) {
            break; // 还没收全 Body，等待下次
        }

        // ==========================================================
        // 走到这里，说明当前 buffer 中确确实实有一个完整的 RPC 数据包
        // ==========================================================

        // 5. 读取 Args 数据
        std::string args_str(data + 4 + header_size, args_size);

        // 6. 核心步骤：确认消息处理无误后，从 buffer 中彻底移除这个包
        //    这会让 readerIndex 向后移动，指向下一个包的开始
        msg->retrieve(4 + header_size + args_size);

        // 7. 开始业务分发
        auto it = serviceMap_.find(service_name);
        if (it == serviceMap_.end())
        {
            std::cout << service_name << " is not exist!" << std::endl;
            continue; // 这里的 continue 是为了处理缓冲区里可能存在的下一个包
        }

        auto mit = it->second.methodMap_.find(method_name);
        if (mit == it->second.methodMap_.end())
        {
            std::cout << service_name << ":" << method_name << " is not exist!" << std::endl;
            continue;
        }

        google::protobuf::Service *service = it->second.service_; 
        const google::protobuf::MethodDescriptor *method = mit->second;

        google::protobuf::Message *request = service->GetRequestPrototype(method).New();
        if (!request->ParseFromString(args_str))
        {
            std::cout << "request parse error, content:" << args_str << std::endl;
            // 虽然业务解析失败，但网络包是完整的，所以还是继续处理下一个
            continue;
        }

        google::protobuf::Message *response = service->GetResponsePrototype(method).New();

        google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider, 
                                                                        const muduo::net::TcpConnectionPtr&, 
                                                                        google::protobuf::Message*>
                                                                        (this, 
                                                                         &RpcProvider::SendRpcResponse, 
                                                                         conn, response);

        service->CallMethod(method, nullptr, request, response, done);
        delete request;
    } });

    //连接zookeeper
    ZkClient zkcli;
    zkcli.Start();

    for(auto &sp:serviceMap_)
    {
        std::string service_path="/"+sp.first;
        zkcli.Create(service_path.data(),nullptr,0);
        for(auto& m:sp.second.methodMap_)
        {
            std::string method_path=service_path+"/"+m.first;
            std::string method_data=ip+":"+std::to_string(port);
            zkcli.Create(method_path.data(),method_data.data(),method_data.size(),ZOO_EPHEMERAL);
        }
    }

    // 设置线程数量，启动网络
    server.setThreadNum(4);
    std::cout << "RpcProvide started with ip: " << ip << " port: " << port << std::endl;
    server.start();
    loop_.loop();
}

void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *responce)
{
    std::string resp;
    if (responce->SerializeToString(&resp))
    {
        // 序列化成功后把结果返回过去，并且主动关闭连接
        conn->send(resp);
    }
    else
    {
        std::cout << "Serialize failed" << std::endl;
    }
    conn->shutdown();
    delete responce;
}