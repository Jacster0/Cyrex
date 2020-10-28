#include "DXException.h"
#include "Core/Utils/StringUtils.h"
#include <comdef.h>
#include <sstream>

Cyrex::DxException::DxException(HRESULT hr, const std::string& functionName, const std::string& file, int line) noexcept
    :
    ErrorCode(hr),
    FunctionName(functionName),
    CyrexException::CyrexException(line,file.c_str())
{
}

const char* Cyrex::DxException::what() const noexcept {
    _com_error err(ErrorCode);
    std::string msg = Cyrex::ToNarrow(err.ErrorMessage());

    return (std::stringstream{} 
        << FunctionName << "failed in " << CyrexException::GetFile() << "\n"
        << "Line: " << CyrexException::GetLine() << "\n"
        << "Error messsage: " << msg).str().c_str();
}

const std::string Cyrex::DxException::GetType() const noexcept {
    return "DirectX Exception";
}
