#include <string>
#include <iomanip>

namespace Cyrex {
    static constexpr auto MAX_STR_CONVERT_SIZE = 512u;

    inline std::string ToNarrow(const std::wstring& wide) {
        char narrow[MAX_STR_CONVERT_SIZE];
        wcstombs_s(nullptr, narrow, wide.c_str(), _TRUNCATE);
     
        return std::string(narrow);
    }

    inline std::wstring ToWide(const std::string& narrow) {
        wchar_t wide[MAX_STR_CONVERT_SIZE];
        mbstowcs_s(nullptr, wide, narrow.c_str(), _TRUNCATE);

        return std::wstring(wide);
    }
}