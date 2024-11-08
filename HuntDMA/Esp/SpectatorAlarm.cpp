#include "Pch.h"
#include "globals.h"
#include "ESPRenderer.h"
#include "ConfigInstance.h"
#include "ConfigUtilities.h"

void DrawSpectators()
{
	EnvironmentInstance->SpectatorCountMutex.lock();
	int spectatorCount = EnvironmentInstance->GetSpectatorCount();
	EnvironmentInstance->SpectatorCountMutex.unlock();

	if (spectatorCount <= 0 || spectatorCount > 11) return;

	// Get screen dimensions
	float screenWidth = ESPRenderer::GetScreenWidth();
	float screenHeight = ESPRenderer::GetScreenHeight();

    // Draw red frame around the screen
    int lineWidth = 50;
    ESPRenderer::DrawRect(
        ImVec2(0, 0),
        ImVec2(screenWidth, screenHeight),
        ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
        static_cast<float>(lineWidth)
    );

    // Draw spectator count
    ESPRenderer::DrawText(
        ImVec2(0, 55),
        std::to_wstring(spectatorCount),
        ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
        40
    );
}