#include "mprpcchannel.h"
#include "google/protobuf/message.h"
#include "rpcheader.pb.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "mprpcapplication.h"
#include "mprpccontroller.h"
#include "zookeeperutil.h"

// headersize + header + args(body)

void myChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                           google::protobuf::RpcController *controller,
                           const google::protobuf::Message *request,
                           google::protobuf::Message *response,
                           google::protobuf::Closure *done)
{
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();

    // 获取参数的序列化字符串长度
    int arg_size = 0;
    std::string arg_str;
    if (request->SerializeToString(&arg_str))
    {
        arg_size = arg_str.size();
    }
    else
    {
        controller->SetFailed("querry failed");
        std::cout << "serialize request failed" << std::endl;
        return;
    }

    // 定义rpc请求头header
    rpcheader::RequreMsg rpcheader;
    rpcheader.set_args_size(arg_size);
    rpcheader.set_service_name(service_name);
    rpcheader.set_method_name(method_name);

    uint32_t headersize;
    std::string header;
    if (rpcheader.SerializeToString(&header))
    {
        headersize = header.size();
        headersize = htonl(headersize);  //防止主机字节序不一致，对方要调用ntohl
    }
    else
    {
        controller->SetFailed("querry failed");
        std::cout << "serialize request failed" << std::endl;
        return;
    }

    // 报文拼接
    std::string msg = std::string(reinterpret_cast<char *>(&headersize), 4) +
                      header + arg_str;
    //这里已经拼接好一个完整的RPC调用报文了


    // 建立连接发起调用;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        controller->SetFailed("querry failed");
        std::cout << "fd 创建失败" << std::endl;
        return;
    }

    //从配置文件找到ip：port
    //std::string ip = MprpcApplication::GetInstance().getconfig().Load("rpcserverip");
    //uint16_t port = stoi(MprpcApplication::GetInstance().getconfig().Load("rpcserverport"));
    
    ZkClient zkcli;
    zkcli.Start();

    std::string method_path = "/" + service_name +"/" + method_name;
    std::string host_data =zkcli.GetData(method_path.data());
    if(host_data=="")
    {
        controller->SetFailed(method_path+"is not exist");
        return;
    }
    int idx = host_data.find(":");
    if(idx==-1)
    {
        controller->SetFailed(method_path+"address is invalid");
        return;
    }
    std::string ip = host_data.substr(0,idx);
    uint16_t port =atoi(host_data.substr(idx+1).c_str());

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.data());

    if (connect(fd, (sockaddr *)&addr, sizeof(addr)))
    {
        controller->SetFailed("querry failed");
        std::cout << "connect failed" << std::endl;
        close(fd);
        return;
    }

    if (send(fd, msg.data(), msg.size(), 0) == -1)
    {
        controller->SetFailed("querry failed");
        std::cout << "send failed" << std::endl;
        close(fd);
        return;
    }

/*
    char buf[1024] = {0};
    int len = recv(fd, buf, 1024, 0);
    if (len == -1)
    {
        std::cout << "recv failed" << std::endl;
        close(fd);
        return;
    }
    std::string response_str(buf, len);
    if (!response->ParseFromString(response_str))
    {
        std::cout << "parse failed" << std::endl;
        close(fd);
        return;
    }
*/
     // 建立 socket ... connect ... send ... (保持不变)

    // 【修改部分】接收响应
    char buf[1024] = {0};
    std::string response_str;
    
    // 循环读取，直到服务端关闭连接 (recv返回0) 或者出错
    while (true)
    {
        int len = recv(fd, buf, sizeof(buf), 0);
        if (len == -1)
        {
            std::cout << "recv error" << std::endl;
            close(fd);
            return;
        }
        if (len == 0)
        {
            // 对端关闭了连接，说明数据发完了
            break;
        }
        // 把收到的数据拼接到 string 里
        response_str.append(buf, len);
    }

    // 此时 response_str 包含了完整的数据
    if (!response->ParseFromString(response_str))
    {
        std::cout << "parse error! response_size:" << response_str.size() << std::endl;
        close(fd);
        return;
    }
    
    close(fd);
}