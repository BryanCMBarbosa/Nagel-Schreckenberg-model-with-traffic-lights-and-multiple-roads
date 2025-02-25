#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <map>
#include <vector>
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
    std::vector<KeyType> getKeys() const;
    void increment(const KeyType &key, const ValueType &incrementValue);

    auto begin() { return data.begin(); }
    auto end() { return data.end(); }
    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }
};

#include "Dictionary.tpp"
#endif
