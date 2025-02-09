#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
class UserService : public fixbug::UserServiceRpc
{
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service : Login !!!" << std::endl;
        std::cout << "name->" << name << " " << "password->" << pwd << std::endl;
        return false;
    }
    /*
    重写基类xxxServiceRpc的虚函数，这些函数都是框架直接调用
    比如：callee 接收到 caller的Login(LoginRequest) 框架就直接交给下面这个重写的Login方法处理了
    */
    void Login(google::protobuf::RpcController *controller,
               const ::fixbug::LoginRequest *request,
               ::fixbug::LoginResponse *response,
               ::google::protobuf::Closure *done)
    {
        // 应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 做本地业务处理
        bool login_result = Login(name, pwd);

        // 把响应写入
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(1);
        code->set_errmsg("测试一下错误信息是否能带回！");
        response->set_sucess(login_result);

        // 执行回调操作   执行响应对象数据的序列化和网络发送（都是由框架来完成的）
        done->Run();
    }
};

int main(int argc, char **argv)
{
    // 调用框架的初始化操作
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象。把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());

    // 启动一个rpc服务发布节点   Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();
    return 0;
}