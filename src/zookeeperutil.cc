#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <iostream>

// 全局的 watcher 观察器，用于处理 ZooKeeper 服务器给客户端的通知
void global_watcher(zhandle_t *zh, int type,
                    int state, const char *path, void *watcherCtx)
{
    // 判断消息类型是否为会话相关的事件
    if (type == ZOO_SESSION_EVENT)
    {
        // 判断连接是否成功
        if (state == ZOO_CONNECTED_STATE)
        {
            sem_t *sem = (sem_t *)zoo_get_context(zh);

            // 发送信号，通知等待的线程 ZooKeeper 已经成功连接
            sem_post(sem);
        }
    }
}

ZkClient::ZkClient() : m_zhandle(nullptr)
{
}
ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle);
    }
}
// zkclient 启动时连接zkserver
void ZkClient::Start()
{
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;
    /*
    zookeeper_mt：表示使用多线程版本的 ZooKeeper 客户端 API
    ZooKeeper 客户端 API 提供了三个线程：
    - API 调用线程：用于处理用户发出的 ZooKeeper API 调用。
    - 网络 I/O 线程：用于处理网络的输入输出，主要通过 `poll` 来进行事件循环。
    - Watcher 回调线程：用于处理 Watcher 事件的回调，`pthread_create` 创建。
    */
    // 初始化 ZooKeeper 客户端，连接到指定的 ZooKeeper 服务
    // 参数说明：
    // connstr.c_str()：ZooKeeper 服务器的连接字符串（格式为 host:port）
    // global_watcher：设置全局的 Watcher 回调函数
    // 30000：超时时间，单位为毫秒
    // nullptr：不传递应用程序数据
    // nullptr：不使用额外的标志
    // 0：客户端版本信息
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 3000, nullptr, nullptr, 0);

    // 检查 初始话是否成功
    if (m_zhandle == nullptr)
    {
        LOG_ERR("zookeeper_init erro!!!");
        exit(EXIT_FAILURE);
    }

    // 信号量
    sem_t sem;
    sem_init(&sem, 0, 0);

    // 将信号量与zookeeper客户端关联
    zoo_set_context(m_zhandle, &sem);

    // 等待信号量，直到 ZooKeeper 完成初始化并向信号量发送信号
    sem_wait(&sem);

    // 初始化成功后输出提示信息
    LOG_INFO("zookeeper_init success!");
    std::cout << "zookeeper_init success!" << std::endl;
}
// 创建节点
void ZkClient::Create_node(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[256];
    int bufferlen = sizeof(path_buffer);
    int flag;
    // 检查指定路径的节点是否存在
    flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (flag == ZNONODE) // 该节点不存在，创建节点
    {
        // 创建
        flag = zoo_create(m_zhandle, path, data, datalen,
                          &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
        
        // 检查 znode 创建是否成功
        if (flag == ZOK)
        {
            std::cout << "znode create success... path:" << path << std::endl;
        }
        else
        {
            std::cout << "flag:" << flag << std::endl;
            std::cout << "znode create error... path:" << path << std::endl;
            exit(EXIT_FAILURE); // 创建失败则退出程序
        }
    }
}
// 获取节点信息
std::string ZkClient::GetData(const char *path)
{
    char buffer[64];
	int bufferlen = sizeof(buffer);
	int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
	if (flag != ZOK)
	{
		std::cout << "get znode error... path:" << path << std::endl;
		return "";
	}
	else
	{
		return buffer;
	}
}