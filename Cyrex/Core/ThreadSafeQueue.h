#pragma once

#include <queue>
#include <mutex>

template<class T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue();
    ThreadSafeQueue(const ThreadSafeQueue& rhs);

    void Push(T value);
    bool TryPop(T& value);
    bool Empty() const;
    size_t Size() const;
private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
};

template<class T>
ThreadSafeQueue<T>::ThreadSafeQueue() { }

template<class T>
ThreadSafeQueue<T>::ThreadSafeQueue(const ThreadSafeQueue& rhs) {
    std::lock_guard<std::mutex> lock(rhs.m_mutex);
    m_queue = rhs.m_queue;
}

template<class T>
void ThreadSafeQueue<T>::Push(T value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(std::move(value));
}

template<class T>
bool ThreadSafeQueue<T>::TryPop(T& value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty()) {
        return false;
    }
    value = m_queue.front();
    m_queue.pop();

    return true;;
}

template<class T>
bool ThreadSafeQueue<T>::Empty() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
}

template<class T>
size_t ThreadSafeQueue<T>::Size() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
}
