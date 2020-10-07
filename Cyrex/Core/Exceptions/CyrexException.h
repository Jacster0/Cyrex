#pragma once
#include <string>
class CyrexException {
public:
    CyrexException(int line, const std::wstring file) noexcept;
    const std::wstring what() const noexcept;
    int GetLine() const noexcept;
    const std::wstring& GetFile() const noexcept;
    std::wstring GetOriginString() const noexcept;
protected:
    virtual const std::wstring GetType() const noexcept;
private:
    int line;
    std::wstring file;
};