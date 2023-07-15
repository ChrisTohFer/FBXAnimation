#pragma once

#include <filesystem>
#include <fstream>
#include <type_traits>

namespace file
{
    std::ofstream open_for_write_binary(const std::filesystem::path& path);
    std::ifstream open_for_read_binary(const std::filesystem::path& path);
}

//serializing=========================================================

template<typename T>
requires std::is_aggregate_v<T>
std::ofstream& operator<<(std::ofstream& stream, const T& value)
{
    const char* data = reinterpret_cast<const char*>(&value);
    stream.write(data, sizeof(T));
    return stream;
}

template<typename T> concept Serializable = requires (T v)
{
    std::ofstream() << v;
};

template<typename T> concept SerializableContainer = requires(T v)
{
    v.begin();
    v.end();
    std::ofstream() << *v.begin();
};

template<typename T>
requires SerializableContainer<T>
std::ofstream& operator<<(std::ofstream& stream, const T& values)
{
    int size = values.size();
    stream << size;
    for (const auto& value : values)
    {
        stream << value;
    }
    return stream;
}

//deserializing=======================================================

template<typename T>
requires std::is_aggregate_v<T>
std::ifstream& operator>>(std::ifstream& stream, T& value)
{
    char* data = reinterpret_cast<char*>(&value);
    stream.read(data, sizeof(T));
    return stream;
}

template<typename T> concept Deserializable = requires (T v)
{
    std::ifstream() >> v;
};

template<typename T> concept DeserializableContainer = requires(T v)
{
    v.begin();
    v.end();
    std::ifstream() >> *v.begin();
};

template<typename T>
requires DeserializableContainer<T>
std::ifstream& operator>>(std::ifstream& stream, T& values)
{
    int size;
    stream >> size;
    values.resize(size);
    for (auto& value : values)
    {
        stream >> value;
    }
    return stream;
}