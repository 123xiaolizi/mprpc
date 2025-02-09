#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

//远程调用方（caller)就是靠这个对象来进行发起远程调用的，可理解为代理
class MprpcChannel : public google::protobuf::RpcChannel
{
public:
    //（重写虚函数）统一做rpc方法调用的数据数据序列化和网络发送
    void CallMethod(const google::protobuf::MethodDescriptor *method,
                    google::protobuf::RpcController *controller, 
                    const google::protobuf::Message *request,
                    google::protobuf::Message *response,
                    google::protobuf::Closure *done);
};