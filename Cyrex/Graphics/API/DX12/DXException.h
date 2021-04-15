#pragma once
#include "Platform/Windows/CrxWindow.h"
#include "Core/Exceptions/CyrexException.h"
#include <source_location>

namespace Cyrex {
    class DxException : public CyrexException {
    public:
        DxException(HRESULT hr, const std::string& functionName, const std::string& file, int line) noexcept;
        const char* what() const noexcept override;
        const std::string GetType() const noexcept override;

        HRESULT ErrorCode = S_OK;
        std::string FunctionName;
    };

    inline void ThrowIfFailed(const HRESULT hr, const std::source_location& loc = std::source_location::current()) {
        if (FAILED(hr)) {
            throw DxException(hr, loc.function_name(), loc.file_name(), loc.line());
        }
    }
}
