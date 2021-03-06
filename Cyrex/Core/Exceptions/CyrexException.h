#pragma once
#include <string>

namespace Cyrex {
    class CyrexException : public std::exception {
    public:
        CyrexException(int line, const std::string file) noexcept;
        virtual const char* what() const noexcept override;
        int GetLine() const noexcept;
        const std::string& GetFile() const noexcept;
        std::string GetOriginString() const noexcept;
        virtual const std::string GetType() const noexcept;
    protected:
        mutable std::string message;
    private:
        int line;
        std::string file;
    };
}
