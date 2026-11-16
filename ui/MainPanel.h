#pragma once

#include "UiPanel.h"

namespace ui
{
class MainPanel final : public UiPanel
{
public:
	void Render(UiState& state) override;
};
}

