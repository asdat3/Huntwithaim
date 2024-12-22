#include "Pch.h"
#include "globals.h"
#include "ConfigInstance.h"
#include "ConfigUtilities.h"
#include "Init.h"
#include "PlayerEsp.h"
#include <algorithm>
#include "ESPRenderer.h"

static void DrawFPS()
{
    ESPRenderer::DrawText(
        ImVec2(25, 25),
        "Overlay FPS: " + std::to_string(FrameRate()),
        Configs.Overlay.FpsColor,
        Configs.Overlay.FpsFontSize
    );
}

static void DrawObjectCount()
{
    ESPRenderer::DrawText(
        ImVec2(25, 25 + Configs.Overlay.FpsFontSize),
        "Objects: " + std::to_string(EnvironmentInstance->GetObjectCount()),
        Configs.Overlay.ObjectCountColor,
        Configs.Overlay.ObjectCountFontSize
    );
}

static void DrawPlayerList()
{
	int y = ESPRenderer::GetScreenHeight() / 2;

    std::vector<std::shared_ptr<WorldEntity>> templist = EnvironmentInstance->GetPlayerList();

    if (templist.empty())
        return;

    std::vector<std::pair<int, std::string>> playerInfoList;

    for (std::shared_ptr<WorldEntity> ent : templist)
    {
        if (!ent || !Configs.Player.DrawFriends && ent->GetType() == EntityType::FriendlyPlayer)
            continue;

        int distance = (int)Vector3::Distance(ent->GetPosition(), CameraInstance->GetPosition());
        if (ent->GetType() == EntityType::LocalPlayer || !ent->GetValid() || ent->IsHidden())
            continue;

        if (!IsValidHP(ent->GetHealth().current_hp) ||
            !IsValidHP(ent->GetHealth().current_max_hp) ||
            !IsValidHP(ent->GetHealth().regenerable_max_hp))
            continue;

        std::string wname = Configs.Player.Name ? ent->GetName() : "";
        std::string wdistance = Configs.Player.Distance ? "[" + std::to_string(distance) + "m]" : "";
        std::string whealth = std::to_string(ent->GetHealth().current_hp) + "/" + std::to_string(ent->GetHealth().current_max_hp) + "[" + std::to_string(ent->GetHealth().regenerable_max_hp) + "]";

        std::string playerInfo = wname + " " + whealth + " " + wdistance;

        playerInfoList.push_back({ distance, playerInfo });
    }

    std::sort(playerInfoList.begin(), playerInfoList.end(), [](const auto& a, const auto& b) {
        return a.first < b.first;
    });

    std::stringstream result;
    for (size_t i = 0; i < playerInfoList.size(); ++i)
    {
        result << playerInfoList[i].second;
        if (i != playerInfoList.size() - 1)
            result << "\n";
    }

    ESPRenderer::DrawText(
        ImVec2(25, y),
        result.str(),
        Configs.Player.PlayerListColor,
        Configs.Player.PlayerListFontSize,
        MiddleLeft
    );
}

static ImVec2 GetCrosshairPosition()
{
    if (Configs.General.CrosshairLowerPosition)
        return ImVec2(ESPRenderer::GetScreenWidth() * 0.5f, ESPRenderer::GetScreenHeight() * 0.6f);
    else
        return ImVec2(ESPRenderer::GetScreenWidth() * 0.5f, ESPRenderer::GetScreenHeight() * 0.5f);
}

static void DrawCrosshair()
{
    ImVec2 center = GetCrosshairPosition();

    if (Configs.Overlay.CrosshairType == 1)
    {
        ESPRenderer::DrawCircle(
            center,
            Configs.Overlay.CrosshairSize,
            Configs.Overlay.CrosshairColor,
            1,
            true
        );
    }
    if (Configs.Overlay.CrosshairType == 2)
    {
        ESPRenderer::DrawCircle(
            center,
            Configs.Overlay.CrosshairSize,
            Configs.Overlay.CrosshairColor,
            1,
            false
        );
    }
    if (Configs.Overlay.CrosshairType == 3)
    {
        ESPRenderer::DrawRect(
            ImVec2(center.x - (Configs.Overlay.CrosshairSize * 0.5f), center.y - (Configs.Overlay.CrosshairSize * 0.5f)),
            ImVec2(center.x + (Configs.Overlay.CrosshairSize * 0.5f), center.y + (Configs.Overlay.CrosshairSize * 0.5f)),
            Configs.Overlay.CrosshairColor,
            1,
            true
        );
    }
    if (Configs.Overlay.CrosshairType == 4)
    {
        ESPRenderer::DrawRect(
            ImVec2(center.x - (Configs.Overlay.CrosshairSize / 2), center.y - (Configs.Overlay.CrosshairSize / 2)),
            ImVec2(center.x + (Configs.Overlay.CrosshairSize / 2), center.y + (Configs.Overlay.CrosshairSize / 2)),
            Configs.Overlay.CrosshairColor,
            1,
            false
        );
    }
}

static void DrawAimbotFOV()
{
    ImVec2 center = GetCrosshairPosition();

    ESPRenderer::DrawCircle(
        center,
        Configs.Aimbot.FOV,
        Configs.Aimbot.FOVColor,
        2,
        false
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

    if (enableAimBot && Configs.Aimbot.DrawFOV)
        DrawAimbotFOV();

    if (Configs.Overlay.CrosshairType != 0) // Lastly draw crosshair to draw it on top of everything
        DrawCrosshair();
}