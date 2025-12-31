#include <iostream>
#include <unistd.h>
#include <sys/inotify.h>
#include <limits.h>
#include <cstring>
#include <map>

// inotify 事件结构体大小 + 文件名最大长度
#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

int main() {
    std::cout << "[LogAgent] 正在启动监控，目标：当前目录(.) ..." << std::endl;

    // 1. 初始化 inotify 实例
    //    这会返回一个文件描述符 fd，你可以把它加入到 epoll 中！
    int fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
        return 1;
    }

    // 2. 添加监控：监控当前目录 "."
    //    IN_CREATE: 文件创建
    //    IN_MODIFY: 文件内容被修改 (Muduo 写日志会触发这个)
    //    IN_MOVED_FROM/TO: 文件重命名 (日志滚动会触发这个)
    //    IN_DELETE: 文件被删除
    int wd = inotify_add_watch(fd, ".", IN_CREATE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO | IN_DELETE);
    if (wd < 0) {
        perror("inotify_add_watch");
        return 1;
    }

    char buffer[BUF_LEN];

    // 3. 死循环处理事件 (在真实 Agent 中，这里会集成到 EventLoop)
    while (true) {
        // read 会阻塞，直到有文件系统事件发生
        // 这里完全不消耗 CPU，直到内核唤醒我们
        int length = read(fd, buffer, BUF_LEN);
        if (length < 0) {
            perror("read");
            break;
        }

        // 4. 解析事件 (可能一次读到多个事件)
        int i = 0;
        while (i < length) {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];

            if (event->len > 0) {
                // event->name 是发生事件的文件名
                std::string filename = event->name;

                // 过滤掉临时文件或隐藏文件
                if (filename[0] != '.') {
                    // 使用并行 if 处理 mask，防止遗漏组合标志，虽然 IN_CREATE/MODIFY 通常互斥
                    if (event->mask & IN_CREATE) {
                        std::cout << "[发现新文件] " << filename << " -> 赶快打开它准备采集!" << std::endl;
                        // 真实逻辑：int new_fd = open(filename); 将 new_fd 加入 epoll 监听列表
                    }
                    
                    if (event->mask & IN_MODIFY) {
                        std::cout << "[文件有新内容] " << filename << " -> 读取新数据发送给 Kafka!" << std::endl;
                        // 真实逻辑：read(file_fd) 读取新增数据
                    }
                    
                    if (event->mask & IN_MOVED_FROM) {
                        // 【关键步骤：日志滚动 - 旧文件处理】
                        // 文件被重命名移走（例如 app.log -> app.log.20231027）
                        std::cout << "[文件被重命名/移走] " << filename << " -> 这是日志滚动的开始" << std::endl;
                        // 真实逻辑：
                        // 1. 找到该文件名对应的 file_fd。
                        // 2. 将其标记为 "Rotating" (待关闭状态)。
                        // 3. 继续读取 file_fd 直到 EOF (读完残留数据)。
                        // 4. 读完后调用 close(file_fd)。
                    }
                    
                    if (event->mask & IN_MOVED_TO) {
                        // 文件被重命名移入（例如 app.log -> app.log.20231027，这是目标名）
                        std::cout << "[文件重命名完成] " << filename << " -> 可能是旧日志归档" << std::endl;
                        // 真实逻辑：通常忽略，因为我们只关心 active 的日志文件名
                    }
                    
                    if (event->mask & IN_DELETE) {
                        std::cout << "[文件被删除] " << filename << " -> 停止采集" << std::endl;
                        // 真实逻辑：立即 close(file_fd)，从 map 中移除
                    }
                }
            }
            // 移动到缓冲区中的下一个事件
            i += (sizeof(struct inotify_event) + event->len);
        }
    }

    close(fd);
    return 0;
}