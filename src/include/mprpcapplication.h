#pragma once
#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"
//mprpc框架初始化类

class MprpcApplication
{
public:
    static void Init(int argc,char *argv[]);
    static MprpcApplication& GetInstance();
    inline static const MprpcConfig& getconfig(){return fileconfig_;}
private:
    MprpcApplication(){};
    MprpcApplication(const MprpcApplication&)=delete;
    MprpcApplication(MprpcApplication&&)=delete;
    static MprpcConfig fileconfig_;
};