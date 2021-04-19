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
#include <Core/InstructionSet/CpuInfo.h>

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

	auto cpuInfo = CPUInfo{}.Info;

	crxlog::log("\nCPU INFO: ",                                 Logger::NewLine(),
		"Active CPU:          ", cpuInfo.BrandString,           Logger::NewLine(),
		"CPU Vendor:          ", cpuInfo.Vendor,                Logger::NewLine(),
		"CPU architecture:    ", cpuInfo.Architecture,          Logger::NewLine(),
		"Cores:               ", cpuInfo.NumCores,              Logger::NewLine(),
		"Logical processors:  ", cpuInfo.NumLogicalProcessors);

	Initialize();
}

Cyrex::Application::~Application() {
	AssimpLogger::Detach();
	Console::Destroy();
}

int Cyrex::Application::Run() {
	if (!m_isInitialized) [[unlikely]] {
		crxlog::critical("Application is not initialized");
	    return 1;
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
	m_window->Kbd.EnableAutorepeat();

	AssimpLogger::Attach();

	m_gfx->Initialize(m_window->GetWidth(), m_window->GetHeight());

	if (!m_gfx->IsInitialized()) [[unlikely]] {
		crxlog::critical("Cannot render to screen because no Graphics object have been intialized");
		m_isInitialized = false;
	    return;
	}

	m_window->SetImGuiIntialized(true);
	m_window->Show();

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
		case KeyCode::Escape:
			if (m_window->CursorEnabled()) {
				m_window->DisableCursor();
				m_window->m_mouse.EnableRawInput();
			}
			else {
				m_window->EnableCursor();
				m_window->m_mouse.DisableRawInput();
			}
			break;
		case KeyCode::F11:
			crxlog::info("Toggled fullscreen mode");
			m_window->ToggleFullScreen(!m_window->FullScreen());
			break;
		case KeyCode::Space:
			m_gfx->AnimateLights() = !m_gfx->AnimateLights();
		}
	}
	
	if (m_window->Kbd.KeyIsPressedOnce(KeyCode::V)) {
		crxlog::info("Toggled VSync");
		m_gfx->ToggleVsync();
	}

	m_gfx->KeyboardInput(m_window->Kbd);
}

void Cyrex::Application::MouseInput() noexcept {
	using mouseEvent = Mouse::Event::Type;
	while (const auto e = m_window->m_mouse.Read()) {
		switch (e->GetType()) {
		case mouseEvent::Move:
			m_gfx->OnMouseMoved(m_window->m_mouse);
			break;
		case mouseEvent::WheelUp:
		case mouseEvent::WheelDown:
			m_gfx->OnMouseWheel(m_window->m_mouse.GetWheelDelta());
			break;
		}
	}

	while (const auto delta = m_window->m_mouse.ReadRawDelta()) {
		if (!m_window->m_mouse.cursor.IsEnabled()) {
			m_gfx->OnMouseMoved(delta->X, delta->Y);
		}
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
