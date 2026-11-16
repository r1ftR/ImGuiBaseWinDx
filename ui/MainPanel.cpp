#include "MainPanel.h"

#include "../third_party/ImGui/imgui.h"

namespace ui
{
namespace
{
void RenderKeyBind(UiState& state, const char* name, unsigned int key, KeyBindTarget target)
{
	ImGui::Text("%s", name);
	ImGui::SameLine(180.0f);

	const char* buttonLabel = target == KeyBindTarget::MenuToggle
		? "Bind##MenuToggle"
		: "Bind##Exit";
	if (ImGui::Button(buttonLabel))
	{
		state.waitingForKey = target;
	}

	ImGui::SameLine(250.0f);
	if (state.waitingForKey == target)
	{
		ImGui::Text("Press a key...");
	}
	else
	{
		ImGui::Text("Key: %u", key);
	}
}
}

void MainPanel::Render(UiState& state)
{
	if (!state.showMainPanel)
	{
		return;
	}

	if (ImGui::Begin("ImGui Base"))
	{
		if (ImGui::BeginTabBar("MainTabs"))
		{
			if (ImGui::BeginTabItem("Config"))
			{
				RenderKeyBind(state, "Menu toggle", state.menuToggleKey, KeyBindTarget::MenuToggle);
				RenderKeyBind(state, "Exit", state.exitKey, KeyBindTarget::Exit);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Functionality"))
			{
				ImGui::Text("Functionality controls will be added here.");
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}

	ImGui::End();
}
}

