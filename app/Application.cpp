#include "Application.h"

#include "../platform/Dx11Renderer.h"
#include "../platform/ImGuiSetup.h"
#include "../platform/Win32Window.h"
#include "../ui/MainPanel.h"

namespace app
{
namespace
{
// ---------------------------------------------------------------
// Change these two functions to customize the base for your project.
// ---------------------------------------------------------------

platform::WindowConfig MakeWindowConfig()
{
	platform::WindowConfig config;
	config.title = L"ImGui Base";
	config.trayTooltip = L"ImGui Base";
	return config;
}

void RegisterFeatures(ui::UiState& state)
{
	// Name, starts enabled, toggle key, waiting for key.
	state.features.push_back({ "Example 1", false, VK_F1, false });
}

// ---------------------------------------------------------------

// Converts a Win32 color (0-255 per channel) to a renderer color (0.0-1.0).
void ToClearColor(COLORREF color, float clearColor[4])
{
	clearColor[0] = GetRValue(color) / 255.0f;
	clearColor[1] = GetGValue(color) / 255.0f;
	clearColor[2] = GetBValue(color) / 255.0f;
	clearColor[3] = 1.0f;
}

void SetMenuVisible(platform::Win32Window& window, ui::UiState& state, bool visible)
{
	state.showMainPanel = visible;
	state.menuToggle.enabled = visible;
	window.Show(visible);
}

// Returns true if the key was used to set a new key binding.
bool TryBindWaitingKey(platform::Win32Window& window, ui::UiState& state, UINT key)
{
	if (state.menuToggle.waitingForKey)
	{
		// Only change the stored key if Windows accepted the new hotkey.
		if (window.SetMenuToggleKey(key))
		{
			state.menuToggle.toggleKey = key;
			state.menuToggle.waitingForKey = false;
		}
		return true;
	}

	for (auto& feature : state.features)
	{
		if (feature.waitingForKey)
		{
			feature.toggleKey = key;
			feature.waitingForKey = false;
			return true;
		}
	}

	return false;
}

void ToggleFeaturesForKey(platform::Win32Window& window, ui::UiState& state, UINT key)
{
	if (state.menuToggle.toggleKey == key)
	{
		SetMenuVisible(window, state, !state.menuToggle.enabled);
	}

	for (auto& feature : state.features)
	{
		if (feature.toggleKey == key)
		{
			feature.enabled = !feature.enabled;
		}
	}
}

void HandleKeyPress(platform::Win32Window& window, ui::UiState& state)
{
	UINT key = 0;
	if (!window.ConsumeKeyPress(key))
	{
		return;
	}

	if (!TryBindWaitingKey(window, state, key))
	{
		ToggleFeaturesForKey(window, state, key);
	}
}
}

int Run(HINSTANCE instance)
{
	const platform::WindowConfig config = MakeWindowConfig();

	platform::Win32Window window;
	if (!window.Initialize(instance, config))
	{
		return -1;
	}

	platform::ImGuiSetup imgui;
	if (!imgui.Initialize(window.GetHandle()))
	{
		return -1;
	}

	// To use another graphics library, replace Dx11Renderer here
	// with your own class that inherits from platform::Renderer.
	platform::Dx11Renderer renderer;
	float clearColor[4];
	ToClearColor(config.transparentColor, clearColor);
	if (!renderer.Initialize(window.GetHandle(), window.GetWidth(), window.GetHeight(), clearColor))
	{
		return -1;
	}

	ui::UiState state;
	state.menuToggle.toggleKey = config.menuToggleKey;
	RegisterFeatures(state);

	ui::MainPanel mainPanel;

	while (window.ProcessMessages())
	{
		HandleKeyPress(window, state);

		if (window.ConsumeExitRequest())
		{
			break;
		}
		else if (window.ConsumeOpenRequest())
		{
			SetMenuVisible(window, state, true);
		}
		else if (window.ConsumeToggleRequest())
		{
			SetMenuVisible(window, state, !state.showMainPanel);
		}

		// While hidden, sleep until Windows sends a new message.
		if (!state.showMainPanel)
		{
			WaitMessage();
			continue;
		}

		UINT width = 0;
		UINT height = 0;
		if (window.ConsumeResize(width, height))
		{
			renderer.Resize(width, height);
		}

		renderer.BeginFrame();
		imgui.BeginFrame();
		mainPanel.Render(state);
		renderer.RenderFrame();

		if (state.exitRequested)
		{
			break;
		}

		// The Menu toggle checkbox may have been changed in the UI.
		if (state.menuToggle.enabled != state.showMainPanel)
		{
			SetMenuVisible(window, state, state.menuToggle.enabled);
		}
	}

	renderer.Shutdown();
	imgui.Shutdown();
	return 0;
}
}
