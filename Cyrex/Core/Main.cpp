#include "Application.h"
#include "Exceptions/CyrexException.h"
#include "Platform/Windows/MessageBox.h"
#include "Graphics/API/DX12/Device.h"
#include <exception>
#include <dxgidebug.h>

#define CRX_ERROR -1

using namespace Cyrex;

int main() {
    try {
#ifdef _DEBUG
        Device::EnableDebugLayer();
#endif 
        return Application{}.Run();
    }
    catch (const CyrexException& e) {
        MessageBox::Show(e.what(), e.GetType(), MessageBox::OK, MessageBox::Exclamation);

        return CRX_ERROR;
    }
    catch (const std::exception& e) {
        MessageBox::Show(e.what(), "Standard Exception", MessageBox::OK, MessageBox::Exclamation);

        return CRX_ERROR;
    }
    catch (...) {
        MessageBox::Show("No details available", "Unknown Exception", MessageBox::OK, MessageBox::Exclamation);

        return CRX_ERROR;
    }
}