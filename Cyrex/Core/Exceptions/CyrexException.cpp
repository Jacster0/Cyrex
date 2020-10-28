#include "CyrexException.h"
#include <sstream>

Cyrex::CyrexException::CyrexException(int line, const std::string file) noexcept
    :
    line(line),
    file(file)
{}

const char* Cyrex::CyrexException::what() const noexcept {
    return (std::stringstream{} << GetType() << "\n" << GetOriginString()).str().c_str();
}

int Cyrex::CyrexException::GetLine() const noexcept {
    return line;
}

const std::string& Cyrex::CyrexException::GetFile() const noexcept {
    return file;
}

std::string Cyrex::CyrexException::GetOriginString() const noexcept {
    return (std::ostringstream{} << "File: " << file << "\n" << "Line: " << line).str();
}

const std::string Cyrex::CyrexException::GetType() const noexcept {
    return "Cyrex Exception";
}
