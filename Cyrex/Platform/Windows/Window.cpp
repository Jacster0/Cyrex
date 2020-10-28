#include "Window.h"
#include "Graphics/Graphics.h"
#include "Core/Application.h"
#include <cassert>

namespace Cyrex {
	Window::WindowClass Window::WindowClass::wndClass;

	Window::WindowClass::WindowClass() noexcept
		:
		hInst(GetModuleHandle(nullptr))
	{
		WNDCLASSEX wc{ 0 };
		wc.cbSize = sizeof(wc);
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = SetupProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = GetInstance();
		wc.hIcon = nullptr;
		wc.hCursor = nullptr;
		wc.hbrBackground = nullptr;
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = L"MainWnd";
		wc.hIconSm = nullptr;

		ATOM atom = RegisterClassEx(&wc);
		assert(atom > 0);
	}

	Window::WindowClass::~WindowClass() {
		UnregisterClass(wndClassName, GetInstance());
	}

	const wchar_t* Window::WindowClass::GetName() noexcept { return wndClassName; }
	HINSTANCE Window::WindowClass::GetInstance() noexcept { return wndClass.hInst; }

	Window::Window(int width, int height, const char* name) noexcept
		:
		m_width(width),
		m_height(height) 
	{
		CreateMainWindow();
	}

	Window::Window() noexcept {
		CreateMainWindow();
	}

	Window::~Window() {
		DestroyWindow(m_hWnd);
	}

	LRESULT Window::SetupProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
		if (msg == WM_NCCREATE) {
			const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
			Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);

			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::RedirectProc));

			return pWnd->MsgProc(hWnd, msg, wParam, lParam);
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	LRESULT Window::RedirectProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
		Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		return pWnd->MsgProc(hWnd, msg, wParam, lParam);
	}

	LRESULT Window::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
		switch (msg) {
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			if (!(lParam & 0x40000000) || Kbd.AutorepeatIsEnabled()) {
				Kbd.OnKeyPressed(static_cast<unsigned char>(wParam));
			}
		break;
		case WM_SYSKEYUP:
		case WM_KEYUP:
			Kbd.OnKeyReleased(static_cast<unsigned char>(wParam));
			break;
		case WM_CHAR:
			Kbd.OnChar(static_cast<unsigned char>(wParam));
			break;
			//We won't handle any Alt+key combination (yet atleast)
		case WM_SYSCHAR:
			break;
		case WM_MOUSEWHEEL:
		{
			//Negative value means we are scrolling down, positive value means we are scrolling up.
			float delta = (static_cast<short>(HIWORD(wParam))) / static_cast<float>(WHEEL_DELTA);
	
			Gfx->OnMouseWheel(delta);
		}
		break;
		case WM_SIZE:
		{
			Resize();
		}
		break;
		case WM_QUIT:
		case WM_DESTROY:
			if (Gfx->IsInitialized()) {
				Application::Get().Flush();
			}
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	void Window::CreateMainWindow() noexcept {
		// Compute window rectangle dimensions based on requested client area dimensions.
		RECT r = { 0, 0, m_width, m_height };
		AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, false);
		int width = r.right - r.left;
		int height = r.bottom - r.top;

		m_hWnd = CreateWindow(
			L"MainWnd",
			Window::WindowClass::GetName(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			width,
			height,
			NULL,
			NULL,
			Window::WindowClass::GetInstance(),
			this);

		if (!m_hWnd) {
			//In the future we might wanna throw an exception here. For now we just return from the function
			//When we get to that point we will also have to remove the noexcept keyword
			return;
		}
	}

	void Window::Resize() const noexcept {
		RECT rect;
		GetClientRect(m_hWnd, &rect);

		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;

		if (Gfx) {
			if (Gfx->IsInitialized()) {
				Gfx->Resize(width, height);
			}
		}
	}

	void Window::ToggleFullScreen(bool fullscreen) noexcept {
		if (m_fullScreen != fullscreen) {
			m_fullScreen = fullscreen;

			if (m_fullScreen) {
				GetWindowRect(m_hWnd, &m_windowRect);

				SetWindowLong(m_hWnd, GWL_STYLE, WS_OVERLAPPED);

				// Query the name of the nearest display device for the window.
				// This is required to set the fullscreen dimensions of the window
				// when using a multi-monitor setup.
				HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
				MONITORINFOEX monitorInfo = {};
				monitorInfo.cbSize = sizeof(MONITORINFOEX);
				GetMonitorInfo(hMonitor, &monitorInfo);

				SetWindowPos(m_hWnd, HWND_TOP,
					monitorInfo.rcMonitor.left,
					monitorInfo.rcMonitor.top,
					monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
					monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
					SWP_FRAMECHANGED | SWP_NOACTIVATE);

				ShowWindow(m_hWnd, SW_MAXIMIZE);
			}
			else {
				SetWindowLong(m_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

				SetWindowPos(m_hWnd, HWND_NOTOPMOST,
					m_windowRect.left,
					m_windowRect.top,
					m_windowRect.right - m_windowRect.left,
					m_windowRect.bottom - m_windowRect.top,
					SWP_FRAMECHANGED | SWP_NOACTIVATE);

				ShowWindow(m_hWnd, SW_NORMAL);
			}
		}
	}
	void Window::Show() noexcept {
		ShowWindow(m_hWnd, SW_SHOW);
		UpdateWindow(m_hWnd);
	}
}