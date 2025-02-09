#include "mprpcconfig.h"
#include <iostream>
// 解析加载配置文件
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    FILE *pf = fopen(config_file,"r");
    if (nullptr == pf)
    {
        std::cout << config_file << " is note exist!" << std::endl;
        exit(EXIT_FAILURE);
    }
    //忽略注释，去掉多余空格
    while(!feof(pf))
    {
        char buf[512] = {0};
        //读一行
        fgets(buf,512,pf);
        std::string read_buf(buf);
        //去掉前后多余空格
        Trim(read_buf);
        //忽略注释
        if (read_buf[0] == '#' || read_buf.empty())
        {
            continue;
        }
        int idx = read_buf.find('=');
        if (idx == -1)
        {
            continue;
        }
        std::string key;
        std::string value;
        key = read_buf.substr(0, idx);
        Trim(key);

        int end_idx = read_buf.find('\n', idx);
        value = read_buf.substr(idx + 1, end_idx-idx-1);
        Trim(value);
        m_configMap.insert({key, value});
    }
    fclose(pf);
}
// 查询配置项信息
std::string MprpcConfig::Load(const std::string &key)
{
    auto it = m_configMap.find(key);
    if (it == m_configMap.end())
    {
        return "";
    }
    return it->second;
}

// 去掉字符串前后的空格
void MprpcConfig::Trim(std::string &src_buf)
{
    int idx = src_buf.find_first_not_of(' ');
    if ( idx != -1)
    {
        //去掉前空格
        src_buf = src_buf.substr(idx, src_buf.size()-idx);//第二个参数是长度
    }
    idx = src_buf.find_last_not_of(' ');
    if (idx != -1)
    {
        //去掉后空格
        src_buf = src_buf.substr(0, idx+1);
    }
}