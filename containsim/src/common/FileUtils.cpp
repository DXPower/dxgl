#include <common/FileUtils.hpp>

#include <format>
#include <fstream>

std::string LoadFileToString(std::string_view path) {
    std::ifstream file{std::string(path), std::ios::binary};

    if (!file.is_open())
        throw std::runtime_error(std::format("Could not open file {}", path));


    std::string contents;

    file.seekg(0, std::ios::end);
    contents.resize((std::size_t) file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&contents[0], contents.size());

    return contents;
}