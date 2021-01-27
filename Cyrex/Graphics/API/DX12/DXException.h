#pragma once
#include "Platform/Windows/CrxWindow.h"
#include "Core/Exceptions/CyrexException.h"

namespace Cyrex {
    class DxException : public CyrexException {
    public:
        DxException(HRESULT hr, const std::string& functionName, const std::string& file, int line) noexcept;
        const char* what() const noexcept override;
        const std::string GetType() const noexcept override;

        HRESULT ErrorCode = S_OK;
        std::string FunctionName;
    };


//Since we want the location information provided from the caller, ThrowIfFailed must now be a macro until  
//we get support for std::source_location in MSVC and Visual Studio
#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                                 \
{                                                                        \
    HRESULT hr = (x);                                                    \
    if(FAILED(hr)) { throw DxException(hr, #x, __FILE__, __LINE__); }    \
}
#endif
}
