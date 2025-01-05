#ifndef LIMITEDQUEUE_TPP
#define LIMITEDQUEUE_TPP

#include "LimitedQueue.h"

template <typename T>
LimitedQueue<T>::LimitedQueue(size_t maxSize) : maxSize_(maxSize) {}

template <typename T>
void LimitedQueue<T>::push(const T& value)
{
    if (queue_.size() == maxSize_)
    {
        queue_.pop_front();
    }
    queue_.push_back(value);
}

template <typename T>
T LimitedQueue<T>::front() const
{
    if (!queue_.empty())
    {
        return queue_.front();
    }
    throw std::runtime_error("Queue is empty");
}

template <typename T>
T LimitedQueue<T>::back() const
{
    if (!queue_.empty())
    {
        return queue_.back();
    }
    throw std::runtime_error("Queue is empty");
}

template <typename T>
bool LimitedQueue<T>::empty() const
{
    return queue_.empty();
}

template <typename T>
size_t LimitedQueue<T>::size() const
{
    return queue_.size();
}

template <typename T>
typename LimitedQueue<T>::iterator LimitedQueue<T>::begin()
{
    return queue_.begin();
}

template <typename T>
typename LimitedQueue<T>::iterator LimitedQueue<T>::end()
{
    return queue_.end();
}

template <typename T>
typename LimitedQueue<T>::const_iterator LimitedQueue<T>::begin() const
{
    return queue_.begin();
}

template <typename T>
typename LimitedQueue<T>::const_iterator LimitedQueue<T>::end() const
{
    return queue_.end();
}

#endif
