#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template<typename T>
class LockQueue
{
public:
    void Push(const T& val)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(val);
        cond_.notify_all();
    }
    T Pop()
    {   
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock,[this]()->bool{return !queue_.empty();});
        T data = queue_.front();
        queue_.pop();
        return data;
    }
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};