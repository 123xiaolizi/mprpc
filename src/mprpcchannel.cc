#include "mprpcchannel.h"
#include "rpcheader.pb.h"
#include "mprpcapplication.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "zookeeperutil.h"
// 将请求打包发送到提供服务方（callee）
// header_size + service_name method_name args_size + args
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller,
                              const google::protobuf::Message *request,
                              google::protobuf::Message *response,
                              google::protobuf::Closure *done)
{
    // 获取服务对象信息
    const google::protobuf::ServiceDescriptor *service_desc = method->service();
    std::string service_name = service_desc->name();
    std::string method_name = method->name();

    // 获取参数的序列化字符串长度
    uint32_t args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        //std::cout << "Serialize error!!!" << std::endl;
        LOG_ERR("Serialize error!!!");
        return;
    }
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    uint32_t rpcHeader_size = 0;
    std::string rpcHeader_str;
    if (rpcHeader.SerializeToString(&rpcHeader_str))
    {
        rpcHeader_size = rpcHeader_str.size();
    }
    else
    {
        //std::cout << "Serialize error!!!" << std::endl;
        LOG_ERR("Serialize error!!!");
        return;
    }
    std::string send_str;
    send_str.insert(0, (char *)&rpcHeader_size, 4);
    send_str += rpcHeader_str;
    send_str += args_str;

    // 打印调试信息
    std::cout << "============================================" << std::endl;
    std::cout << "header_size: " << rpcHeader_size << std::endl;
    std::cout << "rpc_header_str: " << rpcHeader_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "============================================" << std::endl;

    // 使用tcp编程，完成rpc方法的远程调用
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1)
    {
        //std::cout << "create socket error!" << std::endl;
        LOG_ERR("create socket error!");
        return;
    }
    // 获取服务端ip 和 port
    // std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    // uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    // rpc调用方想调用service_name的method_name服务，需要查询zk上该服务所在的host信息
    ZkClient zkCli;
    zkCli.Start();
    //  /UserServiceRpc/Login
    std::string method_path = "/" + service_name + "/" + method_name;
    // 127.0.0.1:8000
    std::string host_data = zkCli.GetData(method_path.c_str());
    if (host_data == "")
    {
        controller->SetFailed(method_path + " is not exist!");
        return;
    }
    int idx = host_data.find(":");
    if (idx == -1)
    {
        controller->SetFailed(method_path + " address is invalid!");
        return;
    }
    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx+1, host_data.size()-idx).c_str()); 

    struct sockaddr_in service_addr;
    service_addr.sin_family = AF_INET;
    service_addr.sin_port = htons(port);
    service_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (-1 == connect(client_fd, (struct sockaddr *)&service_addr, sizeof(service_addr)))
    {
        //std::cout << "connect error!" << std::endl;
        LOG_ERR("connect error!");
        close(client_fd);
        return;
    }

    // 发送rpc请求
    if (-1 == send(client_fd, send_str.c_str(), send_str.size(), 0))
    {
        //std::cout << "send error!" << std::endl;
        LOG_ERR("send error!");
        close(client_fd);
        return;
    }

    // 接收请求响应
    char recv_buf[512];
    int recv_size = 0;
    if (-1 == (recv_size = recv(client_fd, recv_buf, 512, 0)))
    {
        //std::cout << "recv error!" << std::endl;
        LOG_ERR("recv error!");
        close(client_fd);
        return;
    }

    // 将接收到的数据反序列化
    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        //std::cout << "Parse error!" << std::endl;
        LOG_ERR("Parse error!");
        close(client_fd);
        return;
    }
}