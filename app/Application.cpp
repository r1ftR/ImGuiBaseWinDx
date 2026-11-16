#include "Application.h"

#include "../platform/Dx11Renderer.h"
#include "../platform/Win32Window.h"
#include "../ui/MainPanel.h"

namespace app
{
int Run(HINSTANCE instance)
{
	platform::Win32Window window;
	if (!window.Initialize(
		instance,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		L"ImGui Base"))
	{
		return -1;
	}

	platform::Dx11Renderer renderer;
	if (!renderer.Initialize(window.GetHandle(), window.GetWidth(), window.GetHeight()))
	{
		return -1;
	}

	ui::UiState state;
	ui::MainPanel mainPanel;

	while (window.ProcessMessages())
	{
		UINT pressedKey = 0;
		if (state.waitingForKey != ui::KeyBindTarget::None && window.ConsumeKeyPress(pressedKey))
		{
			if (state.waitingForKey == ui::KeyBindTarget::MenuToggle)
			{
				state.menuToggleKey = pressedKey;
				window.SetMenuToggleKey(pressedKey);
			}
			else if (state.waitingForKey == ui::KeyBindTarget::Exit)
			{
				state.exitKey = pressedKey;
				window.SetExitKey(pressedKey);
			}

			state.waitingForKey = ui::KeyBindTarget::None;
		}

		if (window.ConsumeExitRequest())
		{
			break;
		}
		else if (window.ConsumeToggleRequest())
		{
			state.showMainPanel = !state.showMainPanel;
			window.Show(state.showMainPanel);
		}

		UINT width = 0;
		UINT height = 0;
		if (window.ConsumeResize(width, height))
		{
			renderer.Resize(width, height);
		}

		renderer.BeginFrame();
		mainPanel.Render(state);
		renderer.RenderFrame();
	}

	renderer.Shutdown();
	return 0;
}
}
