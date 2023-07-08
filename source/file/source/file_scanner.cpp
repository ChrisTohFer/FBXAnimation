#include "file/file_scanner.h"

namespace file
{
    std::vector<std::filesystem::path> fbx_paths()
    {
        std::vector<std::filesystem::path> result;

        //iterate over all files in the fbx path and return any that have the .fbx extension
        for (auto& dir_entry : std::filesystem::recursive_directory_iterator(g_fbx_path))
        {
            //unsure if this has the correct case-sensitivity
            if (dir_entry.path().extension() == std::filesystem::path(".fbx"))
            {
                result.push_back(dir_entry.path());
            }
        }

        return result;
    }
}