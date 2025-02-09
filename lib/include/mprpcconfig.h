#pragma once

#include <unordered_map>
#include <string>

class MprpcConfig
{
public:

    //解析加载配置文件
    void LoadConfigFile(const char *config_file);
    //查询配置项信息
    std::string Load(const std::string &key);
private:
    //保存配置信息
    std::unordered_map<std::string, std::string>m_configMap;
    // 去掉字符串前后的空格
    void Trim(std::string &src_buf);
};