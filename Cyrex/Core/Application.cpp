#include "Application.h"
#include "Platform/Windows/Window.h"
#include "Logger.h"
#include "Console.h"
#include "Graphics/Graphics.h"
#include "Graphics/API/DX12/DXException.h"
#include "Graphics/API/DX12/CommandQueue.h"
#include "Graphics/API/DX12/Device.h"
#include "Graphics/API/DX12/Adapter.h"
#include "Graphics/API/DX12/Swapchain.h"

using namespace Cyrex;
namespace wrl = Microsoft::WRL;
namespace dx = DirectX;

Cyrex::Application::Application() {
	m_window = std::make_unique<Window>();
	m_gfx    = std::make_shared<Graphics>();

	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	if (Console::Instance()) {
		Console::Show();
	}

	Initialize();
}

Cyrex::Application::~Application() {
	Console::Destroy();
}

int Cyrex::Application::Run() {
	if (m_isInitialized) [[likely]] {
		m_gfx->Initialize(m_window->GetWidth(), m_window->GetHeight());
	
		if (!m_gfx->IsInitialized()) [[unlikely]] {
			crxlog::critical("Cannot render to screen because no Graphics object have been intialized");
		}
	    m_window->Show();
	}
	else [[unlikely]] {
		crxlog::critical("Application is not intialized");
	}

	while (true) {
		if (const auto ecode = MessagePump()) {
			//return exit code
			m_gfx->UnLoadContent();
			return *ecode;
		}
		else {
			//Render/Graphics stuff
			if (m_gfx && m_gfx->IsInitialized()) {
				HandleInput();
				m_gfx->Update();
				m_gfx->Render();
			}
		}
	}
}

void Cyrex::Application::Initialize() noexcept {
	m_window->Gfx = m_gfx;
	m_gfx->SetHwnd(m_window->GetWindowHandle());

	m_isInitialized = true;
}

void Cyrex::Application::HandleInput() noexcept {
	KeyboardInput();
	MouseInput();
}

void Cyrex::Application::KeyboardInput() noexcept {
	while (const auto e = m_window->Kbd.ReadKey()) {
		if (!e->IsPress()) {
			continue;
		}

		switch (e->GetCode()) {
		case VK_ESCAPE:
			if (m_window->CursorEnabled()) {
				m_window->DisableCursor();
				m_window->m_mouse.EnableRawInput();
			}
			else {
				m_window->EnableCursor();
				m_window->m_mouse.DisableRawInput();
			}
			break;
		case VK_F11:
			crxlog::info("Toggled fullscreen mode");
			m_window->ToggleFullScreen(!m_window->FullScreen());
			break;
		}
	}

	if (!m_window->CursorEnabled()) {}

	while (const auto e = m_window->Kbd.ReadChar()) {
		if (e.value() == 'v') {
			crxlog::info("Toggled VSync");
			m_gfx->ToggleVsync();
		}
	}
}

void Cyrex::Application::MouseInput() noexcept {
	while (const auto e = m_window->m_mouse.Read()) {
		switch (e->GetType())
		{
		case Mouse::Event::Type::WheelUp:
			m_gfx->OnMouseWheel(m_window->m_mouse.GetWheelDelta());
			break;
		case Mouse::Event::Type::WheelDown:
			m_gfx->OnMouseWheel(m_window->m_mouse.GetWheelDelta());
			break;
		}
	}

	while (const auto delta = m_window->m_mouse.ReadRawDelta()) {
		if (!m_window->m_mouse.cursor.IsEnabled()) {}
	}
}

std::optional<int> Cyrex::Application::MessagePump() noexcept {
	MSG msg{ 0 };

	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT)
			return msg.wParam;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return {};
}
