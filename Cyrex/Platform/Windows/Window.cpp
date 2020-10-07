#include "Window.h"
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
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	void Window::CreateMainWindow() noexcept {
		// Compute window rectangle dimensions based on requested client area dimensions.
		RECT R = { 0, 0, m_width, m_height };
		AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
		int width = R.right - R.left;
		int height = R.bottom - R.top;

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

		ShowWindow(m_hWnd, SW_SHOW);
		UpdateWindow(m_hWnd);
	}

}