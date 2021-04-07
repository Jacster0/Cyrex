#include "FileSystem.h"
#include <filesystem>
#include <algorithm>

#include "Core/Logger.h"
#include "Core/Utils/StringUtils.h"

namespace fs = std::filesystem;

namespace Cyrex {
    bool FileSystem::IsEmpty(const std::string& str) noexcept {
        //First make sure the string is valid
        if (IsEmptyOrWhiteSpace(str)) {
            return false;
        }
        try {
            return fs::is_empty(str);
        }
        catch (const fs::filesystem_error& e) {
            crxlog::warn(e.what());
        }
        return false;
    }

    std::string FileSystem::RemoveIllegalCharacters(const std::string& str) noexcept {
        std::string legal = str;

        //Remove characters that are illegal for both names and paths
        std::string illegal = ":?\"<>|";

        //Define our predicate
        auto lambda = [&illegal](char c) { return (illegal.find(c) != std::string::npos); };

        legal.erase(std::remove_if(legal.begin(), legal.end(), lambda), legal.end());

        if (IsDirectory(legal)) {
            return legal;
        }

        //Remove slashes which are illegal characters for file names
        illegal = "\\/";
        legal.erase(std::remove_if(legal.begin(), legal.end(), lambda), legal.end());

        return legal;
    }

    std::string FileSystem::ReplaceIllegalCharacters(const std::string& str, char c) noexcept {
        std::string legal = str;

        //Replace characters that are illegal for both names and paths
        std::string illegal = ":?\"<>|";

        //Define our predicate
        auto lambda = [&illegal](char c) { 
            return (illegal.find(c) != std::string::npos);
        };

        std::replace_if(legal.begin(), legal.end(), lambda, c);

        if (IsDirectory(legal)) {
            return legal;
        }

        //Replace slashes which are illegal characters for file names
        illegal = "\\/";
        std::replace_if(legal.begin(), legal.end(), lambda, c);

        return legal;
    }

    bool FileSystem::CreateDirectory(const std::string& path) noexcept {
        try {
            return fs::create_directories(path);
        }
        catch (const fs::filesystem_error& e) {
            crxlog::warn(e.what(), " ", path);
        }
        return false;
    }

    bool FileSystem::Delete(const std::string& path) noexcept {
        try {
            if (Exists(path) && fs::remove_all(path)) {
                return true;
            }
        }
        catch (const fs::filesystem_error& e) {
            crxlog::warn(e.what(), " ", path);
        }
        return false;
    }

    bool FileSystem::Exists(const std::string& path) noexcept {
        try {
            return fs::exists(path);
        }
        catch (const fs::filesystem_error& e) {
            crxlog::warn(e.what(), " ", path);
        }
        return false;
    }

    bool FileSystem::IsDirectory(const std::string& path) noexcept {
        try {
            if (Exists(path) && fs::is_directory(path)) {
                return true;
            }
        }
        catch (const fs::filesystem_error& e) {
            crxlog::warn(e.what(), " ", path);
        }
        return false;
    }

    bool FileSystem::IsFile(const std::string& path) noexcept {
        if (path.empty()) {
            return false;
        }
        try {
            if (Exists(path) && fs::is_regular_file(path)) {
                return true;
            }
        }
        catch (const fs::filesystem_error& e) {
            crxlog::warn(e.what(), " ", path);
        }
        return false;
    }

    bool FileSystem::CopyFile(const std::string& src, const std::string& dst) noexcept {
        if (src == dst) {
            return true;
        }

        if (!Exists(GetDirectoryFromFilePath(dst))) {
            CreateDirectory(GetDirectoryFromFilePath(dst));
        }

        try {
            return fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
        }
        catch (const fs::filesystem_error& e) {
            crxlog::warn(e.what());
            return false;
        }
    }

    bool FileSystem::HasParentPath(const std::string& path) noexcept {
        return fs::path(path).has_parent_path();
    }

    const std::string FileSystem::ReplaceExtension(const std::string& path, const std::string& ext) noexcept {
        return fs::path(path).replace_extension(ext).generic_string();
    }

    const std::string FileSystem::GetDirectoryFromFilePath(const std::string& path) noexcept {
        const size_t lastIndex = path.find_last_of("\\/");

        if (lastIndex != std::string::npos) {
            return path.substr(0, lastIndex + 1);
        }
        return std::string();
    }

    const std::string FileSystem::GetFileNameFromFilePath(const std::string& path) noexcept {
        return fs::path(path).filename().generic_string();
    }

    const std::string FileSystem::GetExtensionFromFilePath(const std::string& path) noexcept {
        std::string ext;

        try {
            ext = fs::path(path).extension().generic_string();
        }
        catch (const fs::filesystem_error& e) {
            crxlog::warn(e.what());
        }
        return ext;
    }

    const std::string FileSystem::GetWorkingDirectory() noexcept{
        return fs::current_path().generic_string();
    }

    const std::string FileSystem::GetRootDirectory(const std::string& path) noexcept {
        return fs::path(path).root_directory().generic_string();
    }

    const std::string FileSystem::GetParentDirectory(const std::string& path) noexcept {
        return fs::path(path).parent_path().generic_string();
    }

    const std::string FileSystem::Append(const std::string& first, const std::string& second) noexcept {
        using namespace std::filesystem;

        const auto firstPath  = path(first);
        const auto SecondPath = path(second);

        return (firstPath / SecondPath).string();
    }

    const std::string FileSystem::ConvertToGenericPath(const std::string& path) noexcept {
        std::string copy = path;
        std::replace(copy.begin(), copy.end(), '\\', '/');

        return copy;
    }

    const FileSystem::DirectoryList FileSystem::GetDirectoriesInDirectory(const std::string& path) noexcept {
        DirectoryList directories;

        const fs::directory_iterator end;

        for (fs::directory_iterator iter(path); iter != end; iter++) {
            if (!fs::is_directory(iter->status())) {
                continue;
            }
            std::string dir;

            try {
                dir = iter->path().string();
            }
            catch (const std::system_error& e) {
                crxlog::warn("Failed to read directory path ", e.what());
            }

            if (!dir.empty()) {
                directories.emplace_back(std::move(dir));
            }
        }

        return directories;
    }

    const FileSystem::FileList FileSystem::GetFilesInDirectory(const std::string& path) noexcept {
        FileList files;

        const fs::directory_iterator end;

        for (fs::directory_iterator iter(path); iter != end; iter++) {
            if (!fs::is_regular_file(iter->status())) {
                continue;
            }
            try {
                files.emplace_back(std::move(iter->path().string()));
            }
            catch (const std::system_error& e) {
                crxlog::warn("Failed to read file path ", e.what());
            }
        }

        return files;
    }
}
