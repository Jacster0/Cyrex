#pragma once
#include <wrl/client.h>
#include <ShObjIdl.h> 
#include <shlwapi.h>

#include <cstdint>
#include <vector>
#include <string>

#include "Core/CommonTypes.h"

namespace Cyrex {
    class OpenFileDialog {
    public:
        OpenFileDialog() noexcept;

        [[nodiscard]] DialogResult Open() noexcept;
        [[nodiscard]] bool GetResult() noexcept { return m_openFileDialog->GetResult(&m_shellItem) >= 0; };

        void SetFilterIndex(uint32_t filterIndex) noexcept;
        [[nodiscard]] const uint32_t GetFilterIndex() const noexcept { return m_filterIndex; }

        void SetFilter(std::vector<COMDLG_FILTERSPEC>&& fileFilters) noexcept;
        [[nodiscard]] const std::vector<COMDLG_FILTERSPEC>& GetFilter()  noexcept { return m_fileFilters; }

        [[nodiscard]] std::wstring GetDisplayName(_SIGDN sigdnName) const noexcept;
        [[nodiscard]] std::wstring GetFilePath() const noexcept;

        void SetOwnerHWND(HWND owner) noexcept { m_ownerHwnd = owner; }
    private:
        Microsoft::WRL::ComPtr<IFileOpenDialog> m_openFileDialog;
        Microsoft::WRL::ComPtr<IShellItem> m_shellItem;
        
        std::vector<COMDLG_FILTERSPEC> m_fileFilters{};

        uint32_t m_filterIndex{};
        bool m_canShowDialog{ false };
        HWND m_ownerHwnd{nullptr};
    };
}
