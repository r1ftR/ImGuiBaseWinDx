#pragma once

#include <Windows.h>
#include <shellapi.h>

namespace platform
{
class Win32Window
{
public:
	~Win32Window();

	bool Initialize(HINSTANCE instance, int width, int height, const wchar_t* title);
	bool ProcessMessages();
	void Shutdown();
	void Show(bool visible);
	bool ConsumeToggleRequest();
	bool ConsumeExitRequest();
	bool ConsumeKeyPress(UINT& key);
	void SetMenuToggleKey(UINT key);
	void SetExitKey(UINT key);

	HWND GetHandle() const { return handle_; }
	UINT GetWidth() const { return width_; }
	UINT GetHeight() const { return height_; }
	bool ConsumeResize(UINT& width, UINT& height);

private:
	bool RegisterHotkeys();
	bool CreateTrayIcon();
	void RemoveTrayIcon();
	void UnregisterHotkeys();
	static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

	HWND handle_ = nullptr;
	UINT width_ = 0;
	UINT height_ = 0;
	bool resized_ = false;
	bool toggleRequested_ = false;
	bool exitRequested_ = false;
	UINT pressedKey_ = 0;
	UINT menuToggleKey_ = VK_INSERT;
	UINT exitKey_ = VK_DELETE;
	NOTIFYICONDATAW trayIcon_{};
	bool trayIconAdded_ = false;
};
}

