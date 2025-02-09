#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class LogQueue
{
public:
    //有多个线程会对logQueue进行操作，需要线程安全
    void Push(const T& data)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(data);
        //通知唤醒一个线程，因为写日志只有一个线程
        m_condition.notify_one();
    }

    //一个线程读queue写日志文件
    T Pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())//如果队列为空，这里就一直为wait状态
        {
            m_condition.wait(lock);
        }
        T data = m_queue.front();
        m_queue.pop();
        return data;
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condition;
};