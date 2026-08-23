//
// Created by Owner on 6/23/2026.
//

#ifndef COGITOPLATFORM_FILE_HPP
#define COGITOPLATFORM_FILE_HPP

#include <fstream>
#include <filesystem>
#include <optional>
#include <functional>


inline std::optional<std::filesystem::path> GetAbsolutePath(const std::filesystem::path& path) {
    if (auto abs_path = std::filesystem::absolute(path); std::filesystem::exists(abs_path))
        return abs_path;
    return {};
}
inline std::optional<std::filesystem::path> GetAbsolutePath(const std::string& path) {
    return GetAbsolutePath(std::filesystem::path(path));
}
inline std::string ReadFile(std::filesystem::path path, bool newLines = false) {
    std::fstream file;
    file.open(GetAbsolutePath(path)->string(), std::fstream::in);

    std::string fileString;
    std::string buffer;
    while (std::getline(file, buffer)) {
        if (!buffer.empty()) fileString += buffer;
        fileString += (newLines) ? "\n" : " ";
    }

    file.close();
    return fileString;
}
inline std::string ReadFile(const std::string& path, bool newLines) {
    return ReadFile(std::filesystem::path(path), newLines);
}

inline std::vector<std::string> ReadLines(const std::filesystem::path& path) {
    std::fstream file;
    file.open(GetAbsolutePath(path)->string(), std::fstream::in);

    std::vector<std::string> lines;
    std::string buffer;
    while (std::getline(file, buffer)) {
        lines.push_back(buffer);
    }
    file.close();
    return lines;
}

inline std::vector<std::string> ReadLines(const std::string& path) {
    return ReadLines(std::filesystem::path(path));
}


// returns false if file exists
inline bool createFile(const std::string& path, std::function<void()> func) {
    if (std::filesystem::exists(path)) {
        return false;
    }
    func();
    return true;
}
// returns false if folder exists
inline bool createFolder(const std::string &path) {
    if (std::filesystem::exists(path)) {
        return false;
    }
    std::filesystem::create_directories(path);
    return true;
}



#endif //COGITOPLATFORM_FILE_HPP
