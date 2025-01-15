#include "Dictionary.h"

template <typename KeyType, typename ValueType>
void Dictionary<KeyType, ValueType>::add(const KeyType &key, const ValueType &value)
{
    data.emplace(key, value);
}

template <typename KeyType, typename ValueType>
ValueType Dictionary<KeyType, ValueType>::get(const KeyType &key) const
{
    auto it = data.find(key);
    if (it != data.end())
        return it->second;
    else
        throw std::runtime_error("Key not found!");
}

template <typename KeyType, typename ValueType>
void Dictionary<KeyType, ValueType>::remove(const KeyType &key)
{
    data.erase(key);
}

template <typename KeyType, typename ValueType>
bool Dictionary<KeyType, ValueType>::isThere(const KeyType &key) const
{
    return data.find(key) != data.end();
}