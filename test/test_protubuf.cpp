#include "test.pb.h"
#include <iostream>
#include <string>
using namespace test_protoc;

void test()
{
    LoginRequest req;
    req.set_name("lxx");
    req.set_pwd("123456");

    //序列化
    std::string send_str;
    if (req.SerializeToString(&send_str))
    {
        std::cout<<send_str.c_str()<<std::endl;
    }
    //反序列化
    LoginRequest req2;
    if (req2.ParseFromString(send_str))
    {
        std::cout << req2.name() << std::endl;
        std::cout << req2.pwd() << std::endl;
    }
}

int main ()
{
    test();

    return 0;
}