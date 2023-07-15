#include "binary_serializer.h"

std::ifstream file::open_for_read_binary(const std::filesystem::path& path)
{
    return std::ifstream(path, std::ios::binary | std::ios::in);
}

std::ofstream file::open_for_write_binary(const std::filesystem::path& path)
{
    return std::ofstream(path, std::ios::binary | std::ios::out);
}
