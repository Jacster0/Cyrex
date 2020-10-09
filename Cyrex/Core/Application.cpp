#include "Application.h"
#include "Platform/Windows/Window.h"
#include "Logger.h"
#include "Console.h"
#include "Graphics/Graphics.h"

using namespace Cyrex;

Cyrex::Application::Application()  :
	m_gfx(std::make_shared<Graphics>()),
	m_wnd(std::make_unique<Window>())
{
	if (Console::Instance()) {
		Console::Show();
	}

	m_wnd->Gfx = m_gfx;
	m_gfx->SetHwnd(m_wnd->GetHWnd());
	m_gfx->Initialize();
}

Cyrex::Application::~Application() = default;

int Cyrex::Application::Run() {
	while (true) {
		if (const auto ecode = MessagePump()) {
			//return exit code
			return *ecode;
		}
		else {
			//Render/Graphics stuff
			HandleInput();
			m_gfx->Update();
			m_gfx->Render();
		}
	}
}

void Cyrex::Application::HandleInput() noexcept {
	KeyboardInput();
}

void Cyrex::Application::KeyboardInput() noexcept {
	while (const auto e = m_wnd->Kbd.ReadKey()) {
		if (!e->IsPress()) {
			continue;
		}

		switch (e->GetCode()) {
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		case VK_F11:
			crxlog::info("Toggled fullscreen mode");
			m_wnd->ToggleFullScreen(!m_wnd->FullScreen());
			break;
		}
	}

	while (const auto e = m_wnd->Kbd.ReadChar()) {
		if (e.value() == 'v') {
			crxlog::info("Toggled VSync");
			m_gfx->ToggleVsync();
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
