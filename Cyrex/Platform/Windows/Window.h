#pragma once
#include <memory>
#include "CrxWindow.h"
#include "Core/Input/Mouse.h"
#include "Core/Input/Keyboard.h"

namespace Cyrex {
	enum class MouseButton {Left, Right};

	class Graphics;
	class Window {
	public:
		Window(int width, int height, const char* name) noexcept;
		Window() noexcept;
		~Window();
		Window(const Window&) = delete;
		Window& operator = (const Window&) = delete;

		HWND GetWindowHandle() const { return m_hWnd; }
		uint32_t GetWidth() const noexcept { return m_width; }
		uint32_t GetHeight() const noexcept { return m_height; }
		void ToggleFullScreen(bool fullscreen) noexcept;
		bool FullScreen() { return m_fullScreen; }
		void Show() noexcept;

		void MouseMove(LPARAM lParam, WPARAM wParam);
		void MouseWheel(LPARAM lParam, WPARAM wParam);
		void MouseDown(LPARAM lParam, MouseButton buttonClicked);
		void MouseUp(LPARAM lParam, MouseButton buttonClicked);
		void RawMouseInput(LPARAM lParam);

		bool CursorEnabled() const noexcept { return m_mouse.cursor.m_cursorEnabled; }
		void EnableCursor() noexcept { m_mouse.cursor.Enable(); }
		void DisableCursor() noexcept { m_mouse.cursor.Disable(m_hWnd); }

		Keyboard Kbd;
		Mouse m_mouse;
		std::shared_ptr<Graphics> Gfx = nullptr;
	private:
		class WindowClass {
		public:
			static const wchar_t* GetName() noexcept;
			static HINSTANCE GetInstance() noexcept;
		private:
			WindowClass();
			~WindowClass();
			WindowClass(const WindowClass&) = delete;
			WindowClass& operator = (const WindowClass&) = delete;
			static constexpr const wchar_t* wndClassName = L"Cyrex Window";
			static WindowClass wndClass;
			HINSTANCE hInst;
		};

		static LRESULT CALLBACK SetupProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		static LRESULT CALLBACK RedirectProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

		void CreateMainWindow() noexcept;
		void Resize() const noexcept;

		HWND m_hWnd;
		RECT m_windowRect;
		int m_width{ 1300 };
		int m_height{ 800 };
		int m_lastMousePosX{};
		int m_lastMousePosY{};
		bool m_fullScreen;
		std::vector<std::byte> m_rawInputBuffer;
	};
}
