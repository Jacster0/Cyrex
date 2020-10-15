#pragma once
#include <memory>
#include "CrxWindow.h"
#include "Core/Input/Mouse.h"
#include "Core/Input/Keyboard.h"

namespace Cyrex {
	class Graphics;
	class Window {
	private:
		class WindowClass {
		public:
			static const wchar_t* GetName() noexcept;
			static HINSTANCE GetInstance() noexcept;
		private:
			WindowClass() noexcept;
			~WindowClass();
			WindowClass(const WindowClass&) = delete;
			WindowClass& operator = (const WindowClass&) = delete;
			static constexpr const wchar_t* wndClassName = L"Cyrex";
			static WindowClass wndClass;
			HINSTANCE hInst;
		};
	public:
		Window(int width, int height, const char* name) noexcept;
		Window() noexcept;
		~Window();
		Window(const Window&) = delete;
		Window& operator = (const Window&) = delete;
	public:
		HWND GetHWnd() const { return m_hWnd; }
		uint32_t GetWidth() const noexcept { return m_width; }
		uint32_t GetHeight() const noexcept { return m_height; }
		void ToggleFullScreen(bool fullscreen) noexcept;
		bool FullScreen() { return m_fullScreen; }
	private:
		static LRESULT CALLBACK SetupProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		static LRESULT CALLBACK RedirectProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	private:
		void CreateMainWindow() noexcept;
	private:
		HWND m_hWnd;
		RECT m_windowRect;
		int m_width{ 800 };
		int m_height{ 600 };
		Mouse m_mouse;
		bool m_fullScreen;
	public:
		Keyboard Kbd;
		std::shared_ptr<Graphics> Gfx = nullptr;
	};
}
