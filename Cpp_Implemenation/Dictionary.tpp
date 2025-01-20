#include "Dictionary.h"

template <typename KeyType, typename ValueType>
void Dictionary<KeyType, ValueType>::add(const KeyType &key, const ValueType &value)
{
    data.insert_or_assign(key, value);
}

template <typename KeyType, typename ValueType>
ValueType Dictionary<KeyType, ValueType>::get(const KeyType &key) const
{
    auto it = data.find(key);
    if (it != data.end())
        return it->second;
    else
        throw std::runtime_error("Key not found in Dictionary!");
}

template <typename KeyType, typename ValueType>
void Dictionary<KeyType, ValueType>::remove(const KeyType &key)
{
    data.erase(key);
}

template <typename KeyType, typename ValueType>
bool Dictionary<KeyType, ValueType>::isThere(const KeyType &key) const
{
    return (data.find(key) != data.end());
}

template <typename KeyType, typename ValueType>
std::vector<KeyType> Dictionary<KeyType, ValueType>::getKeys() const
{
    std::vector<KeyType> keys;
    keys.reserve(data.size());
    for (const auto& [k, v] : data)
    {
        keys.push_back(k);
    }
    return keys;
}

template <typename KeyType, typename ValueType>
void Dictionary<KeyType, ValueType>::increment(const KeyType &key, const ValueType &incrementValue)
{
    if (isThere(key))
    {
        ValueType currentValue = get(key);
        currentValue += incrementValue;
        add(key, currentValue);
    }
    else
    {
        throw std::runtime_error("Key not found in Dictionary! Cannot increment.");
    }
}