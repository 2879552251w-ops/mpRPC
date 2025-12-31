#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include "friend.pb.h"

int main(int argc, char **argv)
{
    // 初始化Rpc框架
    MprpcApplication::Init(argc, argv);
    //==========================================================
    // 桩类所有的方法都是调用callmethod分发任务
    dongxia::UserServiceRpc_Stub stub(new myChannel);
    dongxia::FriendServiceRpc_Stub fstub(new myChannel);
    dongxia::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");

    dongxia::LoginResponse response;
    // rpc调用，会调用mychannel的callmethod进行分发
    stub.Login(nullptr, &request, &response, nullptr);

    // 调用完成开始读
    if (response.result().errcode() == 0)
    {
        std::cout << "调用成功！" << response.success() << std::endl;
    }
    else
    {
        std::cout << "调用失败..." << response.result().errmsg() << std::endl;
    }
    //==========================================================
    dongxia::RegistRequest regrequest;
    request.set_name("zhang san");
    request.set_pwd("123456");

    dongxia::RegistResponse regresponse;
    // rpc调用，会调用mychannel的callmethod进行分发
    stub.Regist(nullptr, &regrequest, &regresponse, nullptr);

    // 调用完成开始读
    if (regresponse.result().errcode() == 0)
    {
        std::cout << "调用成功！" << regresponse.success() << std::endl;
    }
    else
    {
        std::cout << "调用失败..." << regresponse.result().errmsg() << std::endl;
    }

    //==========================================================
    dongxia::GetFriendRequest frequest;
    frequest.set_name("dongxia");
    dongxia::GetFriendResponse fresponse;
    MprpcController fcont;
    // rpc调用，会调用mychannel的callmethod进行分发
    fstub.GetFriend(&fcont, &frequest, &fresponse, nullptr);

    // 调用完成开始读
    if (fcont.Failed())
    {
        std::cout << "调用失败..." << fresponse.result().errmsg() << std::endl;
    }
    else
    {
        if (fresponse.result().errcode() == 0)
        {
            std::cout << frequest.name() << "friend:";
            for (auto &s : fresponse.friendlist())
            {
                std::cout << "  " << s;
            }
            std::cout << std::endl;
        }
        else
        {
            std::cout << "调用失败..." << fresponse.result().errmsg() << std::endl;
        }
    }
    return 0;
}
