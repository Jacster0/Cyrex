import ThreadUtils;
import StringUtils;

#include "Window.h"
#include "Graphics/Graphics.h"
#include "Core/Application.h"
#include "Core/Logger.h"
#include <cassert>
#include <comdef.h>

namespace Cyrex {
	Window::WindowClass Window::WindowClass::wndClass;

	Window::WindowClass::WindowClass()
		:
		hInst(GetModuleHandle(nullptr))
	{
		//Try to initialize the com library
		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		if (FAILED(hr)) [[unlikely]] {
			//It is not a good sign if we are in this branch
			_com_error err(hr);
			auto ErrorMsg = ToNarrow(std::wstring(err.ErrorMessage()));

			crxlog::critical("CoInitialize failed: ", ErrorMsg);
			throw std::exception(ErrorMsg.c_str());
		}

		WNDCLASSEX wc = {};

		wc.cbSize        = sizeof(wc);
		wc.style         = CS_OWNDC;
		wc.lpfnWndProc   = SetupProc;
		wc.cbClsExtra    = 0;
		wc.cbWndExtra    = 0;
		wc.hInstance     = GetInstance();
		wc.hIcon         = nullptr;
		wc.hCursor       = nullptr;
		wc.hbrBackground = nullptr;
		wc.lpszMenuName  = nullptr;
		wc.lpszClassName = GetName();
		wc.hIconSm       = nullptr;

		ATOM atom = RegisterClassExW(&wc);
		assert(atom > 0);
	}

	Window::WindowClass::~WindowClass() {
		UnregisterClassW(wndClassName, GetInstance());
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
		case WM_ACTIVATE:
			if (!m_mouse.cursor.IsEnabled()) {
				if (wParam & WA_ACTIVE) {
					m_mouse.cursor.Confine(m_hWnd);
					m_mouse.cursor.Hide();
				}
				else {
					m_mouse.cursor.Free();
					m_mouse.cursor.Show();
				}
			}
			break;
		case WM_KILLFOCUS:
			Kbd.ClearState();
			break;
		//Keyboard messages
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
			MouseWheel(lParam, wParam);
			break;
		//Mouse messages
		case WM_MOUSEMOVE:
			MouseMove(lParam, wParam);
			break;
		case WM_LBUTTONDOWN:
			MouseDown(lParam, MouseButton::Left);
			break;
		case WM_RBUTTONDOWN:
			MouseDown(lParam, MouseButton::Right);
			break;
		case WM_LBUTTONUP:
			MouseUp(lParam, MouseButton::Left);
			break;
		case WM_RBUTTONUP:
			MouseUp(lParam, MouseButton::Right);
			break;
		//Raw mouse input
		case WM_INPUT: 
			RawMouseInput(lParam);
			break;
		case WM_SIZE:
			Resize();
			break;
		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	void Window::CreateMainWindow() noexcept {
		// Compute window rectangle dimensions based on requested client area dimensions.
		RECT r = { 0, 0, m_width, m_height };
		AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, false);
		int width = r.right - r.left;
		int height = r.bottom - r.top;

		m_hWnd = CreateWindow(
			Window::WindowClass::GetName(),
			L"Cyrex",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			width,
			height,
			nullptr,
			nullptr,
			Window::WindowClass::GetInstance(),
			this);

		if (!m_hWnd) {
			//In the future we might wanna throw an exception here. For now we just return from the function
			//When we get to that point we will also have to remove the noexcept keyword
			return;
		}

		RAWINPUTDEVICE rid;
		rid.usUsagePage = 0x01;
		rid.usUsage = 0x02;
		rid.dwFlags = 0;
		rid.hwndTarget = nullptr;
		
		RegisterRawInputDevices(&rid, 1, sizeof(rid));
	}

	void Window::Resize() const noexcept {
		RECT rect;
		GetClientRect(m_hWnd, &rect);

		int width  = rect.right - rect.left;
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
				monitorInfo.cbSize        = sizeof(MONITORINFOEX);

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
					m_windowRect.right  - m_windowRect.left,
					m_windowRect.bottom - m_windowRect.top,
					SWP_FRAMECHANGED | SWP_NOACTIVATE);

				ShowWindow(m_hWnd, SW_NORMAL);
			}
		}
	}

	void Window::Show() noexcept {
		ShowWindow(m_hWnd, SW_SHOWNORMAL);
		UpdateWindow(m_hWnd);
	}

	void Window::MouseMove(LPARAM lParam, WPARAM wParam) {
		const auto point = MAKEPOINTS(lParam);
		int x = point.x;
		int y = point.y;

		int deltaX = x - m_lastMousePosX;
		int deltaY = y - m_lastMousePosY;

		m_mouse.m_deltaX = deltaX;
		m_mouse.m_deltaY = deltaY;

		m_lastMousePosX = x;
		m_lastMousePosY = y;

		if (!m_mouse.cursor.IsEnabled()) {
			if (!m_mouse.IsInWindow()) {
				SetCapture(m_hWnd);
				m_mouse.OnMouseEnter();
				m_mouse.cursor.Hide();
			}
			return;
		}

		if (x >= 0 && x <= m_width && y >= 0 && y < m_height) {
			m_mouse.OnMouseMove(x,y);

			if (!m_mouse.IsInWindow()) {
				SetCapture(m_hWnd);
				m_mouse.OnMouseEnter();
			}
		}

		else {
			if (wParam & (MK_LBUTTON | MK_RBUTTON)) {
				m_mouse.OnMouseMove(x, y);
			}
			else {
				ReleaseCapture();
				m_mouse.OnMouseLeave();
			}
		}
	}

	void Window::MouseWheel(LPARAM lParam, WPARAM wParam) {
		const auto point = MAKEPOINTS(lParam);
		const int delta  = GET_WHEEL_DELTA_WPARAM(wParam);

		m_mouse.OnWheelDelta(point.x, point.y,delta);
	}

	void Window::MouseDown(LPARAM lParam, MouseButton buttonClicked) {
		const auto point = MAKEPOINTS(lParam);

		if (buttonClicked == MouseButton::Left) {
			SetForegroundWindow(m_hWnd);

			if (!m_mouse.cursor.m_cursorEnabled) {
				m_mouse.cursor.Confine(m_hWnd);
				m_mouse.cursor.Hide();
			}
			m_mouse.OnLeftPressed(point.x, point.y);
		}
		else {
			m_mouse.OnRightPressed(point.x, point.y);
		}
	}

	void Window::MouseUp(LPARAM lParam, MouseButton buttonClicked) {
		const auto point = MAKEPOINTS(lParam);
		int x = point.x;
		int y = point.y;

		(buttonClicked == MouseButton::Left)
			? 
			m_mouse.OnLeftReleased(point.x, point.y)
			: 
			m_mouse.OnRightReleased(point.x, point.y);
		
		if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
			ReleaseCapture();
			m_mouse.OnMouseLeave();
		}
	}

	void Window::RawMouseInput(LPARAM lParam) {
		if (!m_mouse.m_rawEnabled) {
			return;
		}

		uint32_t size;
		auto hRawInput = reinterpret_cast<HRAWINPUT>(lParam);

		//Get the size of the data
		uint32_t result = GetRawInputData(hRawInput, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));

		if (result == -1) {
			return;
		}
		m_rawInputBuffer.resize(size);

		//read the data
		result = GetRawInputData(hRawInput, RID_INPUT, m_rawInputBuffer.data(), &size, sizeof(RAWINPUTHEADER));

		if (result != size) {
			return;
		}

		//Process the data
		const auto& rawInput = reinterpret_cast<const RAWINPUT&>(*m_rawInputBuffer.data());
		int dx = rawInput.data.mouse.lLastX;
		int dy = rawInput.data.mouse.lLastY;

		if (rawInput.header.dwType == RIM_TYPEMOUSE && (dx != 0 || dy != 0)) {
			m_mouse.OnRawDelta(dx, dy);
		}
	}
}