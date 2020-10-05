#include "Application.h"
#include "Logger.h"
#include "Console.h"

using namespace Cyrex;

Cyrex::Application::Application() 
    :
    window(std::make_unique<Window>())
{
	if (Console::Instance()) {
		Console::Show();
	}
}

int Cyrex::Application::Run() {
	while (true) {
		if (const auto ecode = MessagePump()) {
			//return exit code
			return *ecode;
		}
		else {
			//Render/Graphics stuff
		}
	}
}

std::optional<int> Cyrex::Application::MessagePump() {
	MSG msg{ 0 };

	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT)
			return msg.wParam;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return {};
}
