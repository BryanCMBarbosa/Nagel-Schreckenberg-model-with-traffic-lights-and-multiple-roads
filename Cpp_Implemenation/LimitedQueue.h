#ifndef LIMITEDQUEUE_H
#define LIMITEDQUEUE_H

#include <deque>
#include <stdexcept>
#include <iostream>

template <typename T>
class LimitedQueue
{
public:
    explicit LimitedQueue(size_t maxSize);

    void push(const T& value);
    T front() const;
    T back() const;
    bool empty() const;
    size_t size() const;

    using iterator = typename std::deque<T>::iterator;
    using const_iterator = typename std::deque<T>::const_iterator;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

private:
    size_t maxSize_;
    std::deque<T> queue_;
};

#include "LimitedQueue.tpp"
#endif
