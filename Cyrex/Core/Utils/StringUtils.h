#include <string>
#include <iomanip>

namespace Cyrex {
    inline std::string ToNarrow(const std::wstring& wide) {
        char narrow[512];
        wcstombs_s(nullptr, narrow, wide.c_str(), _TRUNCATE);
        return narrow;
    }

    inline std::wstring ToWide(const std::string& narrow) {
        wchar_t wide[512];
        mbstowcs_s(nullptr, wide, narrow.c_str(), _TRUNCATE);
        return wide;
    }
}