#pragma once
#include <string_view>
#include "CrxWindow.h"

//God damn Macros
#ifdef MessageBox
#undef MessageBox
#endif

namespace Cyrex {
    class MessageBox {
    public:
        enum MessageBocIcon {
            None        = 0x0,
            Hand        = 0x10,
            Error       = Hand,
            Stop        = Error,
            Question    = 0x20,
            Exclamation = 0x30,
            Warning     = Exclamation,
            Asterisk    = 0x40,
            Information = Asterisk,
        };

        enum MessageBoxButtons {
            OK               = 0x0,
            OKCancel         = 0x1,
            AbortRetryIgnore = 0x2,
            YesNoCancel      = 0x3,
            YesNo            = 0x4,
            RetryCancel      = 0x5,
        };

        enum MessageBoxDefaultButton {
            Button1 = 0x0,
            Button2 = 0x100,
            Button3 = 0x200,
        };

        enum MessageBoxOptions {
            Default             = 0x0,
            RightAlign          = 0x80000,
            DefaultDesktopOnly  = 0x20000,
            RtlReading          = 0x100000,
            ServiceNotification = 0x200000
        };
        enum class DialogResult {
            None   = 0x0,
            OK     = 0x1,
            Cancel = 0x2,
            Abort  = 0x3,
            Retry  = 0x4,
            Ignore = 0x5,
            Yes    = 0x6,
            No     = 0x7,
        };
    public:
        static DialogResult Show(const std::string_view text) noexcept;
        static DialogResult Show(const std::string_view text, const std::string_view caption) noexcept;
        static DialogResult Show(const std::string_view text, const std::string_view caption, const MessageBoxButtons buttons) noexcept;
        static DialogResult Show(
            const std::string_view text, 
            const std::string_view caption, 
            const MessageBoxButtons buttons,
            const MessageBocIcon icon) noexcept;
        static DialogResult Show(
            const std::string_view text,
            const std::string_view caption,
            const MessageBoxButtons buttons,
            const MessageBocIcon icon,
            const MessageBoxDefaultButton defaultButton) noexcept;
        static DialogResult Show(
            const std::string_view text,
            const std::string_view caption,
            const MessageBoxButtons buttons,
            const MessageBocIcon icon,
            const MessageBoxDefaultButton defaultButton,
            const MessageBoxOptions options) noexcept;
    public:
        static DialogResult Show(const std::wstring_view text) noexcept;
        static DialogResult Show(const std::wstring_view text, const std::wstring_view caption) noexcept;
        static DialogResult Show(const std::wstring_view text, const std::wstring_view caption, const MessageBoxButtons buttons) noexcept;
        static DialogResult Show(
            const std::wstring_view text,
            const std::wstring_view caption,
            const MessageBoxButtons buttons,
            const MessageBocIcon icon) noexcept;
        static DialogResult Show(
            const std::wstring_view text,
            const std::wstring_view caption,
            const MessageBoxButtons buttons,
            const MessageBocIcon icon,
            const MessageBoxDefaultButton defaultButton) noexcept;
        static DialogResult Show(
            const std::wstring_view text,
            const std::wstring_view caption,
            const MessageBoxButtons buttons,
            const MessageBocIcon icon,
            const MessageBoxDefaultButton defaultButton,
            const MessageBoxOptions options) noexcept;
    };
}