#pragma once

#include <filesystem>
#include <fstream>
#include <type_traits>

#include <map>
#include <set>
#include <unordered_map>
#include <vector>

namespace file
{
    std::ofstream open_for_write_binary(const std::filesystem::path& path);
    std::ifstream open_for_read_binary(const std::filesystem::path& path);
}

//serializing=========================================================

template<typename T>
std::ofstream& operator<<(std::ofstream& stream, const T& value)
{
    static_assert(false, "Attempting to serialize a type that isn't serializable.");
    return stream;
}

template<typename T>
requires std::is_trivial_v<T>
std::ofstream& operator<<(std::ofstream& stream, const T& value)
{
    const char* data = reinterpret_cast<const char*>(&value);
    stream.write(data, sizeof(T));
    return stream;
}

template<typename First, typename Second>
std::ofstream& operator<<(std::ofstream& stream, const std::pair<First, Second>& pair)
{
    stream << pair.first << pair.second;
    return stream;
}

template<typename T>
std::ofstream& operator<<(std::ofstream& stream, const std::vector<T>& vec)
{
    int size = vec.size();
    stream << size;
    for (const auto& elem : vec)
    {
        stream << elem;
    }
    return stream;
}

template<typename T>
std::ofstream& operator<<(std::ofstream& stream, const std::set<T>& set)
{
    int size = set.size();
    stream << size;
    for (const auto& elem : set)
    {
        stream << elem;
    }
    return stream;
}

template<typename Key, typename Value>
std::ofstream& operator<<(std::ofstream& stream, const std::map<Key, Value>& map)
{
    int size = map.size();
    stream << size;
    for (const auto& elem : map)
    {
        stream << elem.first << elem.second;
    }
    return stream;
}

template<typename Key, typename Value>
std::ofstream& operator<<(std::ofstream& stream, const std::unordered_map<Key, Value>& map)
{
    int size = map.size();
    stream << size;
    for (const auto& elem : map)
    {
        stream << elem.first << elem.second;
    }
    return stream;
}

//deserializing=======================================================

template<typename T>
std::ifstream& operator>>(std::ifstream& stream, T& value)
{
    static_assert(false, "Attempting to deserialize a type that isn't deserializable.");
    return stream;
}

template<typename T>
requires std::is_trivial_v<T>
std::ifstream& operator>>(std::ifstream& stream, T& value)
{
    char* data = reinterpret_cast<char*>(&value);
    stream.read(data, sizeof(T));
    return stream;
}

template<typename First, typename Second>
std::ifstream& operator>>(std::ifstream& stream, std::pair<First, Second>& pair)
{
    stream >> pair.first >> pair.second;
    return stream;
}

template<typename T>
std::ifstream& operator>>(std::ifstream& stream, std::vector<T>& vec)
{
    int size;
    stream >> size;
    vec.resize(size);
    for (int i = 0; i < size; ++i)
    {
        //stream each element individually, they might have non-trivial structure that would be lost by stream.read()
        stream >> vec[i];
    }
    return stream;
}

template<typename T>
std::ifstream& operator>>(std::ifstream& stream, std::set<T>& set)
{
    int size;
    stream >> size;
    for (int i = 0; i < size; ++i)
    {
        T elem;
        stream >> elem;
        set.emplace(std::move(elem));
    }
    return stream;
}

template<typename Key, typename Value>
std::ifstream& operator>>(std::ifstream& stream, std::map<Key, Value>& map)
{
    int size;
    stream >> size;
    for (int i = 0; i < size; ++i)
    {
        Key key;
        Value value;
        stream >> key >> value;
        map.emplace(std::move(key), std::move(value));
    }
    return stream;
}

template<typename Key, typename Value>
std::ifstream& operator>>(std::ifstream& stream, std::unordered_map<Key, Value>& map)
{
    int size;
    stream >> size;
    for (int i = 0; i < size; ++i)
    {
        Key key;
        Value value;
        stream >> key >> value;
        map.emplace(std::move(key), std::move(value));
    }
    return stream;
}