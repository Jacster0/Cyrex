#include "Application.h"
#include "Exceptions/CyrexException.h"
#include "Platform/Windows/MessageBox.h"
#include <exception>

namespace crx = Cyrex;

int main() {
    try {
        crx::Application::Create();
        return crx::Application::Get().Run();
    }
    catch (const CyrexException& e) {
        crx::MessageBox::Show(e.what(), e.GetType(), crx::MessageBox::OK, crx::MessageBox::Exclamation);
    }
    catch (const std::exception& e) {
        crx::MessageBox::Show(e.what(), "Standard Exception", crx::MessageBox::OK, crx::MessageBox::Exclamation);
    }
    catch (...) {
        crx::MessageBox::Show("No details available", "Unknown Exception", crx::MessageBox::OK, crx::MessageBox::Exclamation);
    }
    return -1;
}