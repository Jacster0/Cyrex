#pragma once

#include <string>
#include <vector>
#include <concepts>

//When cppwin32 has a stable release this won't be a problem anymore
#ifdef CreateDirectory
#undef CreateDirectory
#endif

#ifdef CopyFile
#undef CopyFile
#endif

namespace Cyrex {
    class FileSystem {
    public:
        using FileList = std::vector<std::string>;
        using DirectoryList = std::vector<std::string>;

        [[nodiscard]] static bool IsEmpty(const std::string& str) noexcept;
        [[nodiscard]] static std::string RemoveIllegalCharacters(const std::string& str) noexcept;
        [[nodiscard]] static std::string ReplaceIllegalCharacters(const std::string& str, char c = '_') noexcept;

        [[nodiscard]] static bool CreateDirectory(const std::string& path) noexcept;
        [[nodiscard]] static bool Delete(const std::string& path) noexcept;
        [[nodiscard]] static bool Exists(const std::string& path) noexcept;
        [[nodiscard]] static bool IsDirectory(const std::string& path) noexcept;
        [[nodiscard]] static bool IsFile(const std::string& path) noexcept;
        [[nodiscard]] static bool CopyFile(const std::string& src, const std::string& dst) noexcept;
        [[nodiscard]] static bool HasParentPath(const std::string& path) noexcept;

        [[nodiscard]] static const std::string ReplaceExtension(const std::string& path, const std::string& ext) noexcept;
        [[nodiscard]] static const std::string GetDirectoryFromFilePath(const std::string& path) noexcept;
        [[nodiscard]] static const std::string GetFileNameFromFilePath(const std::string& path) noexcept;
        [[nodiscard]] static const std::string GetExtensionFromFilePath(const std::string& path) noexcept;
        [[nodiscard]] static const std::string GetWorkingDirectory() noexcept;
        [[nodiscard]] static const std::string GetRootDirectory(const std::string& path) noexcept;
        [[nodiscard]] static const std::string GetParentDirectory(const std::string& path) noexcept;
        [[nodiscard]] static const std::string Append(const std::string& first, const std::string& second) noexcept;
        [[nodiscard]] static const std::string ConvertToGenericPath(const std::string& path) noexcept;
        [[nodiscard]] static const DirectoryList GetDirectoriesInDirectory(const std::string& path) noexcept;
        [[nodiscard]] static const FileList GetFilesInDirectory(const std::string& path) noexcept;

        template<typename ...Args>
        [[nodiscard]] static inline std::string AppendMultiple(Args&&... args) noexcept {
            return (std::ostringstream{} << ... << args).str();
        }
    };
}