#pragma once
#include <string>
class CyrexException {
public:
    CyrexException(int line, const std::string file) noexcept;
    const std::string what() const noexcept;
    int GetLine() const noexcept;
    const std::string& GetFile() const noexcept;
    std::string GetOriginString() const noexcept;
protected:
    virtual const std::string GetType() const noexcept;
private:
    int line;
    std::string file;
};