#include "Pch.h"
#include "globals.h"
#include "ConfigInstance.h"
#include "ConfigUtilities.h"
#include "Init.h"
#include "PlayerEsp.h"
#include <algorithm>
#include "ESPRenderer.h"

void DrawFPS()
{
    ESPRenderer::DrawText(
        ImVec2(25, 25),
        L"Overlay FPS: " + std::to_wstring(FrameRate()),
        Configs.Overlay.FpsColor,
        Configs.Overlay.FpsFontSize
    );
}

void DrawObjectCount()
{
    ESPRenderer::DrawText(
        ImVec2(25, 25 + Configs.Overlay.FpsFontSize),
        L"Objects: " + std::to_wstring(EnvironmentInstance->GetObjectCount()),
        Configs.Overlay.ObjectCountColor,
        Configs.Overlay.ObjectCountFontSize
    );
}

void DrawPlayerList()
{
	int y = ESPRenderer::GetScreenHeight() / 2;

    EnvironmentInstance->PlayerListMutex.lock();
    std::vector<std::shared_ptr<WorldEntity>> templist = EnvironmentInstance->GetPlayerList();
    EnvironmentInstance->PlayerListMutex.unlock();

    if (templist.empty())
        return;

    std::vector<std::pair<int, std::wstring>> playerInfoList;

    for (std::shared_ptr<WorldEntity> ent : templist)
    {
        if (!ent || !Configs.Player.DrawFriends && ent->GetType() == EntityType::FriendlyPlayer)
            continue;

        int distance = (int)Vector3::Distance(ent->GetPosition(), CameraInstance->GetPosition());
        if (ent->GetType() == EntityType::LocalPlayer || !ent->GetValid())
            continue;

        if (!IsValidHP(ent->GetHealth().current_hp) || !IsValidHP(ent->GetHealth().current_max_hp) ||
            !IsValidHP(ent->GetHealth().regenerable_max_hp))
            continue;

        std::wstring wname = Configs.Player.Name ? ent->GetName() : L"";
        std::wstring wdistance = Configs.Player.Distance ? L"[" + std::to_wstring(distance) + L"m]" : L"";
        std::wstring whealth = std::to_wstring(ent->GetHealth().current_hp) + L"/" + std::to_wstring(ent->GetHealth().current_max_hp) + L"[" + std::to_wstring(ent->GetHealth().regenerable_max_hp) + L"]";

        std::wstring playerInfo = wname + L" " + whealth + L" " + wdistance;

        playerInfoList.push_back({ distance, playerInfo });
    }

    std::sort(playerInfoList.begin(), playerInfoList.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });

    std::wstringstream result;
    for (size_t i = 0; i < playerInfoList.size(); ++i)
    {
        result << playerInfoList[i].second;
        if (i != playerInfoList.size() - 1)
            result << L"\n";
    }

    ESPRenderer::DrawText(
        ImVec2(25, y),
        result.str(),
        Configs.Player.PlayerListColor,
        Configs.Player.PlayerListFontSize,
        MiddleLeft
    );
}

void DrawOverlay()
{
	if (Configs.Overlay.ShowObjectCount)
		DrawObjectCount();

	if (Configs.Overlay.ShowFPS)
		DrawFPS();

	if (Configs.Player.ShowPlayerList)
		DrawPlayerList();
}