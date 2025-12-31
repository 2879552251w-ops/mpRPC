#include "zookeeperutil.h"
#include "mprpcapplication.h"

void global_watcher(zhandle_t *zh, int type,
                    int state, const char *path, void *watcherCtx)
{
    if(type==ZOO_SESSION_EVENT)
    {
        if(state==ZOO_CONNECTED_STATE)
        {
            sem_t *sem=(sem_t*)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}

ZkClient::ZkClient() : zhandle_(nullptr)
{
}

ZkClient::~ZkClient()
{
    if (zhandle_ != nullptr)
    {
        zookeeper_close(zhandle_);
    }
}

void ZkClient::Start()
{
    std::string host = MprpcApplication::GetInstance().getconfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().getconfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;

    /*
    zookeeper_mt:多线程版本
    zookeeper的API客户端提供三个线程
    API调用线程
    网络IO线程 pthread_create poll
    watcher回调线程
    */
    sem_t sem;
    sem_init(&sem,0,0);
    //初始化并把上下文传进去
    zhandle_ = zookeeper_init(connstr.data(), global_watcher, 30000, nullptr, &sem, 0);
    if (zhandle_ == nullptr)
    {
        std::cout << "Zookeeper Init Failed";
        exit(-1);
    }
    

    sem_wait(&sem);
    std::cout<<"Zookeeper Init Success!"<<std::endl;
}

void ZkClient::Create(const char* path,const char *data,int datalen,int state)
{
    char path_buf[128];
    int flag =zoo_exists(zhandle_,path,0,nullptr);
    if(ZNONODE==flag)
    {
        flag=zoo_create(zhandle_,path,data,datalen,&ZOO_OPEN_ACL_UNSAFE,state,path_buf,sizeof(path_buf));
        if(flag == ZOK)
        {
            std::cout<<"znode create success ... path"<<path<<std::endl;
        }
        else
        {
            std::cout<<"flag:"<<flag<<"\n";
            std::cout<<"znode create error... path:"<<path<<std::endl;
            exit(-1);
        }
    }
}

std::string ZkClient::GetData(const char* path)
{
    char buffer[128];
    int buflen=sizeof(buffer);  
    int flag = zoo_get(zhandle_,path,0,buffer,&buflen,nullptr);
    if(flag != ZOK)
    {
        std::cout<<"get znode error... path:"<<path<<std::endl;
        return "";
    }
    else
    {
        return std::string(buffer,buflen);
    }
}