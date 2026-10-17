#pragma once
#include <vector>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

#include <engine/util/error.h>

namespace engine::util {

template <typename Container = std::vector<std::byte>>
inline Container load_file_as(const fs::path& path) {
    // make sure the file exists and is a regular file
    if(!fs::exists(path)) {
        engine::util::throw_error("File does not exist : " + path.string());
    }
    if(!fs::is_regular_file(path)) {
        engine::util::throw_error("Path does not refer to regular file : " + path.string());
    }

    // try to open file
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if(!file) {
        engine::util::throw_error("Failed to open file : " + path.string());
    }

    // determine file size
    std::streamsize size = file.tellg();
    if(size < 0) {
        engine::util::throw_error("Failed to determine file size : " + path.string());
    }
    file.seekg(0);

    // read bytes
    Container data;
    data.resize(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(data.data()), size)) {
        engine::util::throw_error("Failed to read file : " + path.string());
    }
    return data;
}

inline std::vector<std::byte> load_file(const fs::path& p) {
    return load_file_as<std::vector<std::byte>>(p);
}

inline std::string load_text_file(const fs::path& p) { 
    return load_file_as<std::string>(p);
}

}