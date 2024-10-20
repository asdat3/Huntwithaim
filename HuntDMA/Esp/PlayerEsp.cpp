#include "Pch.h"
#include "PlayerEsp.h"
#include "globals.h"
#include "CheatFunction.h"
#include "Drawing.h"
#include "ConfigInstance.h"
#include <WorldEntity.h>
#include "ConfigUtilities.h"

std::shared_ptr<CheatFunction> UpdatePlayers = std::make_shared<CheatFunction>(5, [] {
	
	EnvironmentInstance->UpdatePlayerList();
});
std::shared_ptr<CheatFunction> UpdateBosses = std::make_shared<CheatFunction>(20, [] {
	EnvironmentInstance->UpdateBossesList();
});

void DrawBosses()
{
	if (EnvironmentInstance == nullptr)
		return;

	if (EnvironmentInstance->GetObjectCount() == 0)
		return;

	if (Configs.Bosses.Enable)
	{
		EnvironmentInstance->BossesListMutex.lock();
		std::vector<std::shared_ptr<WorldEntity>> templist = EnvironmentInstance->GetBossesList();
		EnvironmentInstance->BossesListMutex.unlock();
		if (templist.size() != 0)
		{
			for (std::shared_ptr<WorldEntity> ent : templist)
			{

				if (ent == nullptr)
					continue;
				int distance = (int)Vector3::Distance(ent->GetPosition(), CameraInstance->GetPosition());
				if (distance <= 0)
					continue;
				if (distance > Configs.Bosses.MaxDistance)
					continue;
				if (!ent->GetValid())
					continue;
				Vector2 pos = CameraInstance->WorldToScreen(ent->GetPosition());
				if (pos.x == 0 || pos.y == 0)
					continue;
				std::wstring wname = Configs.Bosses.Name ? ent->GetName() : L"";
				std::wstring wdistance = Configs.Bosses.Distance ? L"[" + std::to_wstring(distance) + L"m]" : L"";
				DrawText(pos.x, pos.y, wname + wdistance, "Verdana", Configs.Bosses.FontSize, Configs.Bosses.TextColour, Center);
			}
		}
	}
}

void DrawPlayers()
{
	if (EnvironmentInstance == nullptr)
		return;
	
	if (EnvironmentInstance->GetObjectCount() == 0)
		return;
	
	if (Configs.Player.Enable || Configs.Player.DrawFrames)
	{
		EnvironmentInstance->PlayerListMutex.lock();
		std::vector<std::shared_ptr<WorldEntity>> templist = EnvironmentInstance->GetPlayerList();
		EnvironmentInstance->PlayerListMutex.unlock();
		if (templist.size() == 0)
			return;
		for (std::shared_ptr<WorldEntity> ent : templist)
		{
			if (ent == nullptr)
				continue;

			auto playerPos = ent->GetPosition();

			if (!Configs.Player.DrawFriends && ent->GetType() == EntityType::FriendlyPlayer)
				continue;

			int distance = (int)Vector3::Distance(playerPos, CameraInstance->GetPosition());
			if (distance <= 0)
				continue;
			if (Vector3::Distance(CameraInstance->GetPosition(), playerPos) <= 2.0f)
				continue;
			if (distance > Configs.Player.MaxDistance)
				continue;

			if (!ent->GetValid())
				continue;

			if (!IsValidHP(ent->GetHealth().current_hp) ||
				!IsValidHP(ent->GetHealth().current_max_hp) ||
				!IsValidHP(ent->GetHealth().regenerable_max_hp))
				continue;

			auto tempPos = playerPos;
			Vector2 lowerFramePos = CameraInstance->WorldToScreen(playerPos, false);
			if (lowerFramePos.IsZero())
				continue;
			
			tempPos.z = playerPos.z + 1.7f;
			Vector2 headPos;
			if (Configs.Player.DrawHeadInFrames)
			{
				headPos = CameraInstance->WorldToScreen(tempPos, false);
				if (headPos.IsZero())
					continue;
			}

			tempPos.z = playerPos.z + 2.0f;
			Vector2 uppderFramePos;
			if (Configs.Player.DrawFrames || Configs.Player.DrawHealthBars)
			{
				uppderFramePos = CameraInstance->WorldToScreen(tempPos, false);
				if (uppderFramePos.IsZero())
					continue;
			}

			tempPos.z = playerPos.z + 2.1f;
			Vector2 healthBarPos;
			if (Configs.Player.DrawHealthBars)
			{
				healthBarPos = CameraInstance->WorldToScreen(tempPos, false);
				if (healthBarPos.IsZero())
					continue;
			}

			Vector2 offset, normal;
			if (Configs.Player.DrawFrames || Configs.Player.DrawHealthBars)
			{
				Vector2 v = uppderFramePos - lowerFramePos;
				float segmentLength = Vector2::Length(v);
				normal = Vector2(-v.y, v.x);
				offset = normal / (2.0f * 2);
			}

			if (Configs.Player.DrawFrames)
			{
				Vector2 A1 = lowerFramePos + offset;
				Vector2 A2 = lowerFramePos - offset;
				Vector2 B1 = uppderFramePos + offset;
				Vector2 B2 = uppderFramePos - offset;

				auto colour = ent->GetType() == EntityType::FriendlyPlayer ? Configs.Player.FriendColour : Configs.Player.FramesColour;

				FilledLineAliased(A1.x, A1.y, A2.x, A2.y, 1, colour);
				FilledLineAliased(A2.x, A2.y, B2.x, B2.y, 1, colour);
				FilledLineAliased(B2.x, B2.y, B1.x, B1.y, 1, colour);
				FilledLineAliased(B1.x, B1.y, A1.x, A1.y, 1, colour);

				if (Configs.Player.DrawHeadInFrames)
				{
					Vector2 headoffset = normal / (6.0f * 2);
					Vector2 Head1 = headPos + offset;
					Vector2 Head2 = headPos + headoffset;
					Vector2 Head3 = headPos - headoffset;
					Vector2 Head4 = headPos - offset;

					FilledLineAliased(Head1.x, Head1.y, Head2.x, Head2.y, 1, colour);
					FilledLineAliased(Head3.x, Head3.y, Head4.x, Head4.y, 1, colour);
				}
			}

			if (Configs.Player.DrawHealthBars)
			{
				auto health = ent->GetHealth();
				Vector2 Health1 = healthBarPos - offset;
				Vector2 Health2 = healthBarPos + offset;
				auto lineHeight = std::max(2.0f, Vector2::Distance(Health1, Health2) / 10.0f);
				float currentHp = health.current_hp / 150.0f;
				float currentMaxHp = health.current_max_hp / 150.0f;
				float potentialMaxHp = health.regenerable_max_hp / 150.0f;
				Vector2 currentHpPos = Vector2(currentHp * Health2.x + (1 - currentHp) * Health1.x, currentHp * Health2.y + (1 - currentHp) * Health1.y);
				Vector2 currentMaxHpPos = Vector2(currentMaxHp * Health2.x + (1 - currentMaxHp) * Health1.x, currentMaxHp * Health2.y + (1 - currentMaxHp) * Health1.y);
				Vector2 potentialMaxHpPos = Vector2(potentialMaxHp * Health2.x + (1 - potentialMaxHp) * Health1.x, potentialMaxHp * Health2.y + (1 - potentialMaxHp) * Health1.y);

				FilledLineAliased(Health1.x, Health1.y, currentHpPos.x, currentHpPos.y, lineHeight, Colour(200, 10, 10)); // current health
				FilledLineAliased(currentHpPos.x, currentHpPos.y, currentMaxHpPos.x, currentMaxHpPos.y, lineHeight, Colour(10, 10, 10)); // regenerable black health
				FilledLineAliased(currentMaxHpPos.x, currentMaxHpPos.y, potentialMaxHpPos.x, potentialMaxHpPos.y, lineHeight, Colour(200, 100, 10)); // burning health
				FilledLineAliased(potentialMaxHpPos.x, potentialMaxHpPos.y, Health2.x, Health2.y, lineHeight, Colour(200, 200, 200)); // lost health
			}

			if (!Configs.Player.Enable)
				continue;

			std::wstring wname = Configs.Player.Name ? ent->GetName() : L"";
			std::wstring wdistance = Configs.Player.Distance ? L"[" + std::to_wstring(distance) + L"m]" : L"";
			std::wstring whealth = Configs.Player.HP ? std::to_wstring(ent->GetHealth().current_hp) + L"/" + std::to_wstring(ent->GetHealth().current_max_hp) + L"[" + std::to_wstring(ent->GetHealth().regenerable_max_hp) + L"]" : L"";
			DrawText(lowerFramePos.x, lowerFramePos.y + 3, wname + wdistance + L"\n" + whealth, "Verdana", Configs.Player.FontSize, ent->GetType() == EntityType::FriendlyPlayer ? Configs.Player.FriendColour : Configs.Player.TextColour, Center);
		}
	}
}

bool IsValidHP(int hp)
{
	return hp > 0 && hp <= 150;
}