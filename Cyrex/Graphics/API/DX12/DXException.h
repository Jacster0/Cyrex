#pragma once
#include "Platform/Windows/CrxWindow.h"
#include "Core/Exceptions/CyrexException.h"

namespace Cyrex {
    inline void ThrowIfFailed(HRESULT hr) {
        if (FAILED(hr)) {
            //TODO: implement proper DirectX exception class
            throw std::exception();
        }
    }
}
