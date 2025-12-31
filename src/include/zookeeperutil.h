#pragma once

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

//封装的zk客户端类
class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    //zkclient启动连接zkserver
    void Start();
    //在zkserver上根据制定的path创建节点
    void Create(const char* path,const char *data,int datalen,int state=0);

    std::string GetData(const char* path);
private:
    zhandle_t* zhandle_;
};