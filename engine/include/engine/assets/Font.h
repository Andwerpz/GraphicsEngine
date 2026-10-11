#pragma once

#include <filesystem>
namespace fs = std::filesystem;

namespace engine::assets {

class Font {
public:
    Font(Font&& other);
    ~Font() = default;

    static Font load_font(const fs::path& path);

private:
    Font();

    
};

}