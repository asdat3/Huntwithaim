#include "Pch.h"
#include "globals.h"
#include "ConfigInstance.h"
#include "ConfigUtilities.h"
#include "Init.h"
#include "PlayerEsp.h"
#include "ESPRenderer.h"
#include "Localization/Localization.h"
#include "ImGuiMenu.h"
#include <algorithm>

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

        std::string wname = Configs.Player.Name ? LOC("entity", ent->GetTypeAsString()) : "";
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

void DrawRadar()
{
	//RYANS RADAR 

	//MAP BOUNDS FOR DEBUGGING 1024X1024 IS CENTER SO MAP IS 2048X2048 
	//BOTTOM RIGHT VOID IS 0,0
	//BOTTOM RIGHT MAP IS 512,512

	std::vector<std::shared_ptr<WorldEntity>> tempPlayerList = EnvironmentInstance->GetPlayerList();

	std::shared_ptr<WorldEntity> LocalPlayer = NULL;
	for (size_t index = 0; index < tempPlayerList.size(); ++index)
	{
		std::shared_ptr<WorldEntity> ent = tempPlayerList[index];
		if (ent == nullptr)
			continue;
		if (ent->GetType() == EntityType::LocalPlayer)
		{
			LocalPlayer = ent;
			break;
		}
	}

	if (LocalPlayer == NULL)
		return;

	if (Configs.Overlay.DrawRadar) {
		if (ImGuiUtils::IsKeyPressed(VK_TAB)) {

			// Screen dimensions
			float screenWidth;
			float screenHeight;

			if (Configs.General.OverrideResolution) {
				screenWidth = static_cast<float>(Configs.General.Width);
				screenHeight = static_cast<float>(Configs.General.Height);
			}
			else {
				screenWidth = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
				screenHeight = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));
			}

			// MiniMap Size
			float MapSizeMagicalNumber = 0.6493056f;
			float MapSize = MapSizeMagicalNumber * screenHeight;

			// Radar center on the screen
			float mapCenterX = screenWidth / 2.0f;
			float mapCenterY = screenHeight / 2.0f;

			// World Bounds
			float worldMinX = 512.0f, worldMaxX = 1535.0f;
			float worldMinY = 512.0f, worldMaxY = 1535.0f;

			// Offset for map
			float horizontaloffset = (screenWidth * 0.001171875) * -1.0; //This gets the offset through screenspace of the monitor and * -1 to get negative offset using standard 16:9
			float verticaloffset = screenHeight * 0.0090277; //This gets the offset through screenspace height using standard 16:9 aspect ratio

			// Draw MiniMap Background (Rectangle Outline)
			ESPRenderer::DrawRect(
				ImVec2(mapCenterX - MapSize / 2 + horizontaloffset, mapCenterY - MapSize / 2 + verticaloffset),
				ImVec2(mapCenterX + MapSize / 2 + horizontaloffset, mapCenterY + MapSize / 2 + verticaloffset),
				ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
				1.0f, false
			);

			// Local Player Position
			float playerWorldX = LocalPlayer->GetPosition().x;
			float playerWorldY = LocalPlayer->GetPosition().y;

			// Normalize Local Player Position to MiniMap Space
			float playerMapX = ((playerWorldX - worldMinX) / (worldMaxX - worldMinX)) * MapSize;
			float playerMapY = ((worldMaxY - playerWorldY) / (worldMaxY - worldMinY)) * MapSize;

			// Convert MiniMap Position to Screen Space
			float playerScreenX = mapCenterX - (MapSize / 2) + playerMapX + horizontaloffset;
			float playerScreenY = mapCenterY - (MapSize / 2) + playerMapY + verticaloffset;

			// Draw Local Player
			if (Configs.Overlay.RadarDrawSelf) {

				ESPRenderer::DrawCircle(ImVec2(playerScreenX, playerScreenY), 5.0f, Configs.Overlay.PlayerRadarColor, 1.0f, true);
			}

			// Draw Enemies
			if (tempPlayerList.size() == 0)
				return;
			for (std::shared_ptr<WorldEntity> ent : tempPlayerList)
			{
				if (ent == nullptr || ent->GetType() == EntityType::LocalPlayer)
					continue;

				auto playerPos = ent->GetPosition();

				if (!Configs.Player.DrawFriends && ent->GetType() == EntityType::FriendlyPlayer)
					continue;

				if (!ent->GetValid() || ent->IsHidden()) // Has extracted
					continue;

				auto isDead = false;
				if (ent->GetType() != EntityType::FriendlyPlayer &&
					(!IsValidHP(ent->GetHealth().current_hp) ||
						!IsValidHP(ent->GetHealth().current_max_hp) ||
						!IsValidHP(ent->GetHealth().regenerable_max_hp)))
				{
					ent->SetType(EntityType::DeadPlayer);
					isDead = true;
				}
				else
					ent->SetType(EntityType::EnemyPlayer);

				if (isDead)
					continue;

				float enemyWorldX = playerPos.x;
				float enemyWorldY = playerPos.y;

				// Normalize enemy position 
				float enemyMapX = ((enemyWorldX - worldMinX) / (worldMaxX - worldMinX)) * MapSize;
				float enemyMapY = ((worldMaxY - enemyWorldY) / (worldMaxY - worldMinY)) * MapSize;

				// Convert MiniMap Position to Screen Space
				float enemyScreenX = mapCenterX - (MapSize / 2) + enemyMapX + horizontaloffset;
				float enemyScreenY = mapCenterY - (MapSize / 2) + enemyMapY + verticaloffset;

				ESPRenderer::DrawCircle(ImVec2(enemyScreenX, enemyScreenY), 5.0f, Configs.Overlay.EnemyRadarColor, 1.0f, true);
			}
		}
	}
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