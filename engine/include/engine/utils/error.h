#pragma once
#include <source_location>
#include <stdexcept>
#include <string>

namespace engine::utils {

[[noreturn]]
inline void throw_error(
    const std::string& message,
    std::source_location location = std::source_location::current()
) {
    throw std::runtime_error(
        std::string(location.file_name()) + ":" +
        std::to_string(location.line()) + ": " +
        message
    );
}

}