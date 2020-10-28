#include "CyrexException.h"
#include <sstream>

CyrexException::CyrexException(int line, const std::string file) noexcept
    :
    line(line),
    file(file)
{}

const std::string CyrexException::what() const noexcept {
    return (std::stringstream{} << GetType() << "\n" << GetOriginString()).str();
}

int CyrexException::GetLine() const noexcept {
    return line;
}

const std::string& CyrexException::GetFile() const noexcept {
    return file;
}

std::string CyrexException::GetOriginString() const noexcept {
    return (std::ostringstream{} << "File: " << file << "\n" << "Line: " << line).str();
}

const std::string CyrexException::GetType() const noexcept {
    return "Cyrex Exception";
}
