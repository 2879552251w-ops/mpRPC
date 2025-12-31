#include "mprpcconfig.h"
#include <fstream>
#include <iostream>

// 辅助函数：去除字符串首尾空格
void Trim(std::string &s)
{
    if (s.empty()) return;

    // 去掉头部空格
    s.erase(0, s.find_first_not_of(" \n\r\t"));
    // 去掉尾部空格
    s.erase(s.find_last_not_of(" \n\r\t") + 1);
}

// 载入配置文件
void MprpcConfig::LoadConfigFile(const std::string& configfile)
{
    std::ifstream fin(configfile);
    if (!fin.is_open())
    {
        std::cerr << configfile << " is not exist!" << std::endl;
        // 不要直接 exit，这会导致无法单元测试或上层无法捕获
        // 可以选择 throw 异常，或者仅仅 return 让后续逻辑判断 map 为空
        return; 
    }
    
    std::string line;
    while (std::getline(fin, line))
    {
        // 1. 先对整行去首尾空格，防止空行里只有空格的情况
        Trim(line);

        // 2. 判断注释或空行
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        // 3. 查找等号
        size_t idx = line.find('=');
        if (idx == std::string::npos)
        {
            // 解析错误也可以选择跳过，而不是直接挂掉程序
            continue; 
        }

        // 4. 分割 key 和 value
        std::string key = line.substr(0, idx);
        std::string value = line.substr(idx + 1);

        // 5. 关键步骤：再次去除 key 和 value 两端的空格
        // 这样支持 "rpc_port = 8000" 这种写法
        Trim(key);
        Trim(value);

        // 6. 存入 map
        config_[key] = value;
    }

    fin.close();
}
// 查询配置文件
const std::string MprpcConfig::Load(const std::string &key) const
{
    auto it=config_.find(key);
    if(it==config_.end())
    {
        return std::string();
    }
    return it->second;
}