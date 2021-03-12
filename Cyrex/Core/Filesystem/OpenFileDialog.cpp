#include "OpenFileDialog.h"

#include <array>

using namespace Cyrex;
using namespace Microsoft::WRL;

Cyrex::OpenFileDialog::OpenFileDialog() noexcept {
    auto hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&m_openFileDialog));

    m_canShowDialog = SUCCEEDED(hr);
}

DialogResult OpenFileDialog::Open() noexcept {
    auto dialogResult = DialogResult::None;

    if (m_canShowDialog) {
        if (SUCCEEDED(m_openFileDialog->Show(m_ownerHwnd))) {
            if (GetResult()) {
                dialogResult = DialogResult::OK;
            }           
        }
    }
    return dialogResult;
}

void Cyrex::OpenFileDialog::SetFilterIndex(uint32_t filterIndex) noexcept {
    m_filterIndex = filterIndex;
    m_openFileDialog->SetFileTypeIndex(m_filterIndex);
}

void Cyrex::OpenFileDialog::SetFilter(std::vector<COMDLG_FILTERSPEC>&& fileFilters) noexcept {
    m_fileFilters = std::move(fileFilters);
    m_openFileDialog->SetFileTypes(m_fileFilters.size(), m_fileFilters.data());
}

std::wstring Cyrex::OpenFileDialog::GetDisplayName(_SIGDN sigdnName) const noexcept {
     PWSTR displayName;
    if (m_shellItem->GetDisplayName(sigdnName, &displayName) >= 0) {
        std::wstring res = std::wstring(displayName);
        CoTaskMemFree(displayName);

        return res;
    }
    return std::wstring();
}

std::wstring Cyrex::OpenFileDialog::GetFilePath() const noexcept {
    return GetDisplayName(SIGDN_FILESYSPATH);
}
