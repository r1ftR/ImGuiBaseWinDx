#include "Win32Window.h"

#include "../third_party/ImGui/backends/imgui_impl_win32.h"

#pragma comment(lib, "Shell32.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wParam,
	LPARAM lParam);

namespace platform
{
namespace
{
constexpr wchar_t kWindowClassName[] = L"ImGuiBaseWinDxWindow";
constexpr UINT kTrayIconId = 1;
constexpr UINT kTrayCallbackMessage = WM_APP + 1;
constexpr UINT kMenuToggleHotkeyId = 1;
constexpr UINT kExitHotkeyId = 2;
constexpr UINT kTrayExitCommand = 1001;
}

Win32Window::~Win32Window()
{
	Shutdown();
}

bool Win32Window::Initialize(HINSTANCE instance, int width, int height, const wchar_t* title)
{
	width_ = static_cast<UINT>(width);
	height_ = static_cast<UINT>(height);

	WNDCLASSEXW windowClass{};
	windowClass.cbSize = sizeof(WNDCLASSEXW);
	windowClass.hInstance = instance;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.lpszClassName = kWindowClassName;
	windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);

	if (!RegisterClassExW(&windowClass) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
	{
		return false;
	}

	handle_ = CreateWindowExW(
		WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		kWindowClassName,
		title,
		WS_POPUP,
		0,
		0,
		width,
		height,
		nullptr,
		nullptr,
		instance,
		this);

	if (!handle_)
	{
		return false;
	}

	SetLayeredWindowAttributes(handle_, RGB(0, 0, 0), 0, LWA_COLORKEY);

	if (!RegisterHotkeys() || !CreateTrayIcon())
	{
		Shutdown();
		return false;
	}

	Show(true);
	return true;
}

void Win32Window::Shutdown()
{
	if (!handle_)
	{
		return;
	}

	RemoveTrayIcon();
	UnregisterHotkeys();
	DestroyWindow(handle_);
	handle_ = nullptr;
}

void Win32Window::Show(bool visible)
{
	if (!handle_)
	{
		return;
	}

	ShowWindow(handle_, visible ? SW_SHOW : SW_HIDE);
	if (visible)
	{
		UpdateWindow(handle_);
	}
}

bool Win32Window::RegisterHotkeys()
{
	if (!RegisterHotKey(handle_, kMenuToggleHotkeyId, MOD_NOREPEAT, menuToggleKey_))
	{
		return false;
	}

	if (!RegisterHotKey(handle_, kExitHotkeyId, MOD_NOREPEAT, exitKey_))
	{
		UnregisterHotKey(handle_, kMenuToggleHotkeyId);
		return false;
	}

	return true;
}

bool Win32Window::CreateTrayIcon()
{
	trayIcon_.cbSize = sizeof(trayIcon_);
	trayIcon_.hWnd = handle_;
	trayIcon_.uID = kTrayIconId;
	trayIcon_.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	trayIcon_.uCallbackMessage = kTrayCallbackMessage;
	trayIcon_.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
	wcscpy_s(trayIcon_.szTip, L"ImGui Base");

	trayIconAdded_ = Shell_NotifyIconW(NIM_ADD, &trayIcon_) == TRUE;
	return trayIconAdded_;
}

void Win32Window::RemoveTrayIcon()
{
	if (!trayIconAdded_)
	{
		return;
	}

	Shell_NotifyIconW(NIM_DELETE, &trayIcon_);
	trayIconAdded_ = false;
}

void Win32Window::UnregisterHotkeys()
{
	if (handle_)
	{
		UnregisterHotKey(handle_, kMenuToggleHotkeyId);
		UnregisterHotKey(handle_, kExitHotkeyId);
	}
}

bool Win32Window::ConsumeToggleRequest()
{
	const bool requested = toggleRequested_;
	toggleRequested_ = false;
	return requested;
}

bool Win32Window::ConsumeExitRequest()
{
	const bool requested = exitRequested_;
	exitRequested_ = false;
	return requested;
}

bool Win32Window::ConsumeKeyPress(UINT& key)
{
	if (pressedKey_ == 0)
	{
		return false;
	}

	key = pressedKey_;
	pressedKey_ = 0;
	return true;
}

void Win32Window::SetMenuToggleKey(UINT key)
{
	UnregisterHotKey(handle_, kMenuToggleHotkeyId);
	menuToggleKey_ = key;
	RegisterHotKey(handle_, kMenuToggleHotkeyId, MOD_NOREPEAT, menuToggleKey_);
}

void Win32Window::SetExitKey(UINT key)
{
	UnregisterHotKey(handle_, kExitHotkeyId);
	exitKey_ = key;
	RegisterHotKey(handle_, kExitHotkeyId, MOD_NOREPEAT, exitKey_);
}

bool Win32Window::ProcessMessages()
{
	MSG message{};
	while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
	{
		if (message.message == WM_QUIT)
		{
			return false;
		}

		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	return true;
}

bool Win32Window::ConsumeResize(UINT& width, UINT& height)
{
	if (!resized_)
	{
		return false;
	}

	width = width_;
	height = height_;
	resized_ = false;
	return true;
}

LRESULT CALLBACK Win32Window::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui::GetCurrentContext() && ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam))
	{
		return true;
	}

	auto* instance = reinterpret_cast<Win32Window*>(GetWindowLongPtrW(window, GWLP_USERDATA));

	if (message == WM_NCCREATE)
	{
		auto* createData = reinterpret_cast<CREATESTRUCTW*>(lParam);
		instance = static_cast<Win32Window*>(createData->lpCreateParams);
		SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(instance));
		instance->handle_ = window;
	}

	if (instance)
	{
		if (message == WM_HOTKEY && wParam == kMenuToggleHotkeyId)
		{
			instance->toggleRequested_ = true;
		}
		else if (message == WM_HOTKEY && wParam == kExitHotkeyId)
		{
			instance->exitRequested_ = true;
		}
		else if (message == WM_KEYDOWN && (lParam & 0x40000000) == 0)
		{
			instance->pressedKey_ = static_cast<UINT>(wParam);
		}
		else if (message == WM_SIZE && wParam != SIZE_MINIMIZED)
		{
			instance->width_ = LOWORD(lParam);
			instance->height_ = HIWORD(lParam);
			instance->resized_ = true;
		}
		else if (message == kTrayCallbackMessage &&
			(lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU))
		{
			HMENU menu = CreatePopupMenu();
			AppendMenuW(menu, MF_STRING, kTrayExitCommand, L"Exit");

			POINT cursorPosition{};
			GetCursorPos(&cursorPosition);
			SetForegroundWindow(window);
			TrackPopupMenu(
				menu,
				TPM_RIGHTBUTTON,
				cursorPosition.x,
				cursorPosition.y,
				0,
				window,
				nullptr);
			DestroyMenu(menu);
			PostMessageW(window, WM_NULL, 0, 0);
		}
		else if (message == WM_COMMAND && LOWORD(wParam) == kTrayExitCommand)
		{
			instance->exitRequested_ = true;
		}
		else if (message == WM_DESTROY)
		{
			instance->RemoveTrayIcon();
			instance->UnregisterHotkeys();
			PostQuitMessage(0);
		}
		else if (message == WM_NCDESTROY)
		{
			SetWindowLongPtrW(window, GWLP_USERDATA, 0);
			instance->handle_ = nullptr;
		}
	}

	return DefWindowProcW(window, message, wParam, lParam);
}
}

