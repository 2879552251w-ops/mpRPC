#include "mprpcapplication.h"
#include <unistd.h>
#include <iostream>
#include <string>

void ShowHelp()
{
    printf("format: command -i <condigdile>\n");
}

MprpcConfig MprpcApplication::fileconfig_;
void MprpcApplication::Init(int argc, char *argv[])
{
    if(argc<=2)
    {
        ShowHelp();
        exit(EXIT_FAILURE);
    }
    int c=0;
    std::string config_file;
    while((c=getopt(argc,argv,"i:"))!=-1)
    {
        switch(c)
        {
        case 'i':
                config_file=optarg;
                break;
        case '?':
            ShowHelp();
            exit(EXIT_FAILURE);
        case ':':
            ShowHelp();
            exit(EXIT_FAILURE);
        default:
            ShowHelp();
            exit(EXIT_FAILURE);
        }
    }

    //开始加载配置文件 rpcserver_ip= rpcserver_port= zookeeper_ip= zookeeper_port =
    fileconfig_.LoadConfigFile(config_file);

    std::cout<<fileconfig_.Load("rpcserverip")<<"\n";
    std::cout<<fileconfig_.Load("rpcserverport")<<"\n";
    std::cout<<fileconfig_.Load("zookeeperip")<<"\n";
    std::cout<<fileconfig_.Load("zookeeperport")<<std::endl;

}


MprpcApplication &MprpcApplication::GetInstance()
{
    static MprpcApplication mprpcapp;
    return mprpcapp;
}