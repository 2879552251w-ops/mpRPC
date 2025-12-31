#include "logger.h"

Logger& Logger::GetInstance()
{
    static Logger logger;
    return logger;
}

Logger::Logger():run_(true)
{
    std::thread writeLogTask([&]{
        while(run_)
        {
            //获取当前日期
            time_t now = time(0);
            tm *nowtm = localtime(&now);

            char file_name[128];
            sprintf(file_name,"%d-%d-%d-log.txt",nowtm->tm_year+1900,nowtm->tm_mon+1,nowtm->tm_mday);
            FILE *pf=fopen(file_name,"a+");
            if(pf==nullptr)
            {
                perror("file open"); exit(-1);
            }
            char buf[128]={0};
            sprintf(buf,"%d-%d-%d=>%s ",nowtm->tm_year+1900,nowtm->tm_mon+1,nowtm->tm_mday,(loglevel_==INFO ? "INFO":"ERRO"));
            std::string msg = lockque_.Pop();
            msg.insert(0,buf);
            msg.append("\n");
            fputs(msg.data(),pf);
            
            fclose(pf);
        }
    });
    writeLogTask.detach();
}

void Logger::SetLogLevel(LogLevel level)
{
    loglevel_=level;
}

void Logger::Log(std::string msg)
{
    lockque_.Push(msg);
}