#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <map>
#include <string>
#include <stdexcept>
#include <iostream>

template <typename KeyType, typename ValueType>
class Dictionary
{
private:
    std::map<KeyType, ValueType> data;

public:
    void add(const KeyType &key, const ValueType &value);
    ValueType get(const KeyType &key) const;
    void remove(const KeyType &key);
    bool isThere(const KeyType &key) const;
};

#include "Dictionary.tpp"
#endif
