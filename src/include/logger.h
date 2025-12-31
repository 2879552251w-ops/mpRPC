#pragma once
#include "lockqueue.h"

enum LogLevel
{
    INFO,
    ERROR,
};

//Mprpc框架日志系统
class Logger
{
public:
    static Logger& GetInstance();
    void SetLogLevel(LogLevel level);
    void Log(std::string msg);
private:
    int loglevel_;
    bool run_;
    std::thread writeLogTask;
    LockQueue<std::string> lockque_;
    Logger();
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
};

#define LOG_INFO(logmsgformat,...) \
    do \
    {   \
        Logger &logger = Logger::GetInstance();\
        logger.SetLogLevel(INFO); \
        char c[1024] = {0}; \
        snprintf(c,1024,logmsgformat,##__VA_ARGS__);\
        logger.Log(c); \
    }while(0)

#define LOG_ERR(logmsgformat,...) \
    do \
    {   \
        Logger &logger = Logger::GetInstance();\
        logger.SetLogLevel(ERROR); \
        char c[1024] = {0}; \
        snprintf(c,1024,logmsgformat,##__VA_ARGS__);\
        logger.Log(c); \
    }while(0)