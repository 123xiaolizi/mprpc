#pragma once
#include <zookeeper/zookeeper.h>
#include <semaphore.h>
#include <string>

//封装zk客户端类
class ZkClient
{

public:
    ZkClient();
    ~ZkClient();
    // zkclient 启动时连接zkserver
    void Start();
    // 创建节点
    void Create_node(const char* path, const char* data, int datalen, int state = 0);//默认创建永久节点
    //获取节点信息
    std::string GetData(const char* path);
private:
    // zk的客户端句柄
    zhandle_t *m_zhandle;
};
