#pragma once

#include <unordered_map>
#include <string>

//框架读取配置文件 rpcserver_ip= rpcserver_port= zookeeper_ip= zookeeper_port =
class MprpcConfig
{
public:
    //载入配置文件
    void LoadConfigFile(const std::string& configfile);
    //查询配置文件
    const std::string Load(const std::string &key) const;
private:
    std::unordered_map<std::string,std::string> config_;
};