#include "MessageBox.h"

Cyrex::DialogResult Cyrex::MessageBox::Show(const std::string_view text) noexcept {
    return Show(text, "Error");
}

Cyrex::DialogResult Cyrex::MessageBox::Show(const std::string_view text, const std::string_view caption) noexcept {
    return Show(text, caption, MessageBoxButtons::OKCancel);
}

Cyrex::DialogResult Cyrex::MessageBox::Show(
    const std::string_view text,
    const std::string_view caption, 
    const MessageBoxButtons buttons) noexcept
{
    return Show(text, caption, buttons, MessageBocIcon::Error);
}

Cyrex::DialogResult Cyrex::MessageBox::Show(
    const std::string_view text, 
    const std::string_view caption, 
    const MessageBoxButtons buttons,
    const MessageBocIcon icon) noexcept
{
    return Show(text, caption, buttons, icon, MessageBoxDefaultButton::Button1);
}

Cyrex::DialogResult Cyrex::MessageBox::Show(
    const std::string_view text, 
    const std::string_view caption, 
    const MessageBoxButtons buttons, 
    const MessageBocIcon icon, 
    const MessageBoxDefaultButton defaultButton) noexcept
{
    return Show(text, caption, buttons, icon, defaultButton, MessageBoxOptions::Default);
}

Cyrex::DialogResult Cyrex::MessageBox::Show(
    const std::string_view text, 
    const std::string_view caption, 
    const MessageBoxButtons buttons, 
    const MessageBocIcon icon, 
    const MessageBoxDefaultButton defaultButton, 
    const MessageBoxOptions options) noexcept
{
    return static_cast<Cyrex::DialogResult>(
        MessageBoxA(nullptr, text.data(), caption.data(), buttons | icon | defaultButton | options));
}

Cyrex::DialogResult Cyrex::MessageBox::Show(const std::wstring_view text) noexcept {
    return Show(text, L"Error");
}

Cyrex::DialogResult Cyrex::MessageBox::Show(const std::wstring_view text, const std::wstring_view caption) noexcept {
    return Show(text, caption, MessageBoxButtons::OKCancel);
}

Cyrex::DialogResult Cyrex::MessageBox::Show(
    const std::wstring_view text,
    const std::wstring_view caption,
    const MessageBoxButtons buttons) noexcept
{
    return Show(text, caption, buttons, MessageBocIcon::Error);
}

Cyrex::DialogResult Cyrex::MessageBox::Show(
    const std::wstring_view text,
    const std::wstring_view caption,
    const MessageBoxButtons buttons,
    const MessageBocIcon icon) noexcept
{
    return Show(text, caption, buttons, icon, MessageBoxDefaultButton::Button1);
}

Cyrex::DialogResult Cyrex::MessageBox::Show(
    const std::wstring_view text,
    const std::wstring_view caption,
    const MessageBoxButtons buttons,
    const MessageBocIcon icon,
    const MessageBoxDefaultButton defaultButton) noexcept
{
    return Show(text, caption, buttons, icon, defaultButton, MessageBoxOptions::Default);
}

Cyrex::DialogResult Cyrex::MessageBox::Show(
    const std::wstring_view text,
    const std::wstring_view caption,
    const MessageBoxButtons buttons,
    const MessageBocIcon icon,
    const MessageBoxDefaultButton defaultButton,
    const MessageBoxOptions options) noexcept
{
    return static_cast<Cyrex::DialogResult>(
        MessageBoxW(nullptr, text.data(), caption.data(), buttons | icon | defaultButton | options));
}
