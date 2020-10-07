#include "CyrexException.h"
#include <sstream>

CyrexException::CyrexException(int line, const std::wstring file) noexcept
    :
    line(line),
    file(file)
{}

const std::wstring CyrexException::what() const noexcept {
    return (std::wstringstream{} << GetType() << "\n" << GetOriginString()).str();
}

int CyrexException::GetLine() const noexcept {
    return line;
}

const std::wstring& CyrexException::GetFile() const noexcept {
    return file;
}

std::wstring CyrexException::GetOriginString() const noexcept {
    return (std::wostringstream{} << L"File: " << file << "\n" << L"Line: " << line).str();
}

const std::wstring CyrexException::GetType() const noexcept {
    return L"Cyrex Exception";
}
