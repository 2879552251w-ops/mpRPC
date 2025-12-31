#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "friend.pb.h"

// 使用在RPC服务发布端
class UserService : public dongxia::UserServiceRpc
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service:Login:\n";
        std::cout << "name:" << name << " pwd:" << pwd << std::endl;
        return true;
    }
    // 重写基类UserServiceRpc虚函数
    void Login(::google::protobuf::RpcController *controller,
               const ::dongxia::LoginRequest *request,
               ::dongxia::LoginResponse *response,
               ::google::protobuf::Closure *done)
    {
        // 接收参数
        const std::string &name = request->name(); // 直接常引用接收
        std::string pwd = request->pwd();          // 深拷贝

        // 调用本地方法
        bool result = Login(name, pwd);

        // 写返回值
        response->set_success(result);
        dongxia::ResultCode *code = response->mutable_result();
        code->set_errcode(1);
        code->set_errmsg("我故意的");

        // 执行回调
        done->Run();
    }

    bool Regist(std::string name, std::string pwd)
    {
        std::cout << "doing local service:Regist:\n";
        std::cout << "name:" << name << " pwd:" << pwd << std::endl;
        return true;
    }
    void Regist(::google::protobuf::RpcController *controller,
                const ::dongxia::RegistRequest *request,
                ::dongxia::RegistResponse *response,
                ::google::protobuf::Closure *done)
    {
        // 接收参数
        const std::string &name = request->name(); // 直接常引用接收
        std::string pwd = request->pwd();          // 深拷贝

        // 调用本地方法
        bool result = Regist(name, pwd);

        // 写返回值
        response->set_success(result);
        dongxia::ResultCode *code = response->mutable_result();
        code->set_errcode(1);
        code->set_errmsg("注册失败...");

        // 执行回调
        done->Run();
    }
};

class FriendService : public dongxia::FriendServiceRpc
{
public:
    std::vector<std::string> GetFriend(std::string name)
    {
        std::cout << "寻找数据库有无这个人" << std::endl;
        std::cout << "注册成功" << std::endl;
        return std::vector<std::string>({"zhang san", "li si", "gao yang"});
    }
    void GetFriend(::google::protobuf::RpcController *controller,
                   const ::dongxia::GetFriendRequest *request,
                   ::dongxia::GetFriendResponse* response,
                   ::google::protobuf::Closure *done)
    {
        std::string friendname = request->name();

        auto re = GetFriend(friendname);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("添加成功!");
        for(auto& name:re)
        {
            response->add_friendlist(name);
        }

        done->Run();
    }
};

#include "logger.h"
int main(int argc, char *argv[])
{

    LOG_INFO("nihao");
    LOG_ERR("nihao");
    // 初始化框架
    MprpcApplication::Init(argc, argv);
    // 定义发布服务对象
    RpcProvider provider;
    // 注册自定义服务
    provider.NotifyService(new UserService());
    provider.NotifyService(new FriendService());
    // 启动一个rpc服务发布节点，阻塞等待远程rpc请求
    provider.Run();
    return 0;
}