#include <muduo/base/AsyncLogging.h>
#include <muduo/base/Logging.h>

// 定义滚动大小：1GB
static const off_t kRollSize = 1024 * 1024 * 1024;

// 全局的异步日志对象指针
muduo::AsyncLogging* g_asyncLog = nullptr;

// 这是一个回调函数：muduo::Logger 产生日志后，会调用它
void asyncOutput(const char* msg, int len)
{
    g_asyncLog->append(msg, len);
}

int main(int argc, char* argv[])
{
    // 1. 设置文件名 (例如: "RpcServer")
    // 2. 设置滚动大小 (kRollSize)
    // 3. 这里的 "RpcServer" 是日志文件名的前缀，生成的日志类似于 RpcServer.20231228-120000.log
    muduo::AsyncLogging log("RpcServer", kRollSize);
    
    // 4. 将全局指针指向它 
    g_asyncLog = &log;

    // 5. 【核心步骤】修改 Logger 的默认输出目的地
    //    默认是输出到 stdout，这里改成了调用我们的 asyncOutput
    muduo::Logger::setOutput(asyncOutput);

    // 6. 启动后台日志线程（开始往磁盘写）
    log.start();

    // ------------------------------------------------
    // 你的业务代码，比如 RpcProvider.Run() ...
    // ------------------------------------------------
    
    LOG_INFO << "Server started, logging to file..."; // 这条日志会进入文件

    return 0;
}