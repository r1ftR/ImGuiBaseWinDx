#pragma once

namespace ui
{
constexpr unsigned int kDefaultMenuToggleKey = 0x2D;
constexpr unsigned int kDefaultExitKey = 0x2E;

enum class KeyBindTarget
{
	None,
	MenuToggle,
	Exit
};

struct UiState
{
	bool showMainPanel = true;
	KeyBindTarget waitingForKey = KeyBindTarget::None;
	unsigned int menuToggleKey = kDefaultMenuToggleKey;
	unsigned int exitKey = kDefaultExitKey;
};
}

