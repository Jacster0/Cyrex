#include "MessageBox.h"

Cyrex::MessageBox::DialogResult Cyrex::MessageBox::Show(const std::string_view text) noexcept {
    return Show(text, "Error");
}

Cyrex::MessageBox::DialogResult Cyrex::MessageBox::Show(const std::string_view text, const std::string_view caption) noexcept {
    return Show(text, caption, MessageBoxButtons::OKCancel);
}

Cyrex::MessageBox::DialogResult Cyrex::MessageBox::Show(
    const std::string_view text,
    const std::string_view caption, 
    const MessageBoxButtons buttons) noexcept
{
    return Show(text, caption, buttons, MessageBocIcon::Error);
}

Cyrex::MessageBox::DialogResult Cyrex::MessageBox::Show(
    const std::string_view text, 
    const std::string_view caption, 
    const MessageBoxButtons buttons,
    const MessageBocIcon icon) noexcept
{
    return Show(text, caption, buttons, icon, MessageBoxDefaultButton::Button1);
}

Cyrex::MessageBox::DialogResult Cyrex::MessageBox::Show(
    const std::string_view text, 
    const std::string_view caption, 
    const MessageBoxButtons buttons, 
    const MessageBocIcon icon, 
    const MessageBoxDefaultButton defaultButton) noexcept
{
    return Show(text, caption, buttons, icon, defaultButton, MessageBoxOptions::Default);
}

Cyrex::MessageBox::DialogResult Cyrex::MessageBox::Show(
    const std::string_view text, 
    const std::string_view caption, 
    const MessageBoxButtons buttons, 
    const MessageBocIcon icon, 
    const MessageBoxDefaultButton defaultButton, 
    const MessageBoxOptions options) noexcept
{
    return static_cast<Cyrex::MessageBox::DialogResult>(
        MessageBoxA(nullptr, text.data(), caption.data(), buttons | icon | defaultButton | options));
}

Cyrex::MessageBox::DialogResult Cyrex::MessageBox::Show(const std::wstring_view text) noexcept {
    return Show(text, L"Error");
}

Cyrex::MessageBox::DialogResult Cyrex::MessageBox::Show(const std::wstring_view text, const std::wstring_view caption) noexcept {
    return Show(text, caption, MessageBoxButtons::OKCancel);
}

Cyrex::MessageBox::DialogResult Cyrex::MessageBox::Show(
    const std::wstring_view text,
    const std::wstring_view caption,
    const MessageBoxButtons buttons) noexcept
{
    return Show(text, caption, buttons, MessageBocIcon::Error);
}

Cyrex::MessageBox::DialogResult Cyrex::MessageBox::Show(
    const std::wstring_view text,
    const std::wstring_view caption,
    const MessageBoxButtons buttons,
    const MessageBocIcon icon) noexcept
{
    return Show(text, caption, buttons, icon, MessageBoxDefaultButton::Button1);
}

Cyrex::MessageBox::DialogResult Cyrex::MessageBox::Show(
    const std::wstring_view text,
    const std::wstring_view caption,
    const MessageBoxButtons buttons,
    const MessageBocIcon icon,
    const MessageBoxDefaultButton defaultButton) noexcept
{
    return Show(text, caption, buttons, icon, defaultButton, MessageBoxOptions::Default);
}

Cyrex::MessageBox::DialogResult Cyrex::MessageBox::Show(
    const std::wstring_view text,
    const std::wstring_view caption,
    const MessageBoxButtons buttons,
    const MessageBocIcon icon,
    const MessageBoxDefaultButton defaultButton,
    const MessageBoxOptions options) noexcept
{
    return static_cast<Cyrex::MessageBox::DialogResult>(
        MessageBoxW(nullptr, text.data(), caption.data(), buttons | icon | defaultButton | options));
}
