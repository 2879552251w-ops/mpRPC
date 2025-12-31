#include "test.pb.h"
#include <iostream>
#include <string>
using namespace std;
using namespace dongxia;

int main1()
{
    dongxia::LoginRequest req;
    req.set_name("zhang san");
    req.set_pwd("123456");
    string seril;
    if(req.SerializeToString(&seril))
    cout<<seril<<endl;
    LoginRequest reqb;
    if(reqb.LoginRequest::ParseFromString(seril))
    {
        cout<<reqb.name()<<":"<<reqb.pwd()<<endl;
    }

    
    return 0;
}

int main()
{
    LoginResponse rsp;
    ResultCode *rc = rsp.mutable_result();
    rc->set_errcode(1);
    rc->set_errmsg("login error...");
    string msg;
    if(rsp.SerializeToString(&msg))
    {
        cout<<msg<<endl;
    }

    GetFriendListsResponse friend_;
    ResultCode *cd = friend_.mutable_result();
    cd->set_errcode(0);
    cd->set_errmsg("ok");

    user *fr=friend_.add_friend_list();
    fr->set_age(18);
    fr->set_name("li si");
    fr->set_sex(user::MAN);

    fr=friend_.add_friend_list();
    fr->set_age(18);
    fr->set_name("li si");
    fr->set_sex(user::MAN);
    friend_.add_friend_list()->set_age(10);
    cout<<friend_.friend_list_size()<<endl;
}