#include "Pch.h"
#include "PlayerEsp.h"
#include "globals.h"
#include "CheatFunction.h"
#include "ESPRenderer.h"
#include "ConfigInstance.h"
#include <WorldEntity.h>
#include "ConfigUtilities.h"
#include "Localization/Localization.h"

void DrawSupply()
{
	std::vector<std::shared_ptr<WorldEntity>> templist = EnvironmentInstance->GetSupplyList();

	if (templist.size() == 0)
		return;

	for (std::shared_ptr<WorldEntity> ent : templist)
	{
		if (ent == nullptr)
			continue;

		auto type = ent->GetType();
		if (!Configs.Supply.ShowAmmoSwapBox &&
			(type == EntityType::AmmoSwapBox ||
				type == EntityType::SpitzerBullets ||
				type == EntityType::PoisonBullets ||
				type == EntityType::HighVelocityBullets ||
				type == EntityType::IncendiaryBullets ||
				type == EntityType::DumDumBullets ||
				type == EntityType::ExplosiveBullets ||
				type == EntityType::FMJBullets ||
				type == EntityType::PoisonBolts ||
				type == EntityType::ExplosiveBolts ||
				type == EntityType::IncendiaryShells ||
				type == EntityType::PennyShotShells ||
				type == EntityType::FlechetteShells ||
				type == EntityType::SlugShells))
			continue;
		if (!Configs.Supply.ShowCompactAmmo && (type == EntityType::CompactAmmoPackage))
			continue;
		if (!Configs.Supply.ShowMediumAmmo && (type == EntityType::MediumAmmoPackage))
			continue;
		if (!Configs.Supply.ShowLongAmmo && (type == EntityType::LongAmmoPackage))
			continue;
		if (!Configs.Supply.ShowShortgunAmmo && (type == EntityType::ShotgunAmmoPackage))
			continue;
		if (!Configs.Supply.ShowSpecialAmmo && (type == EntityType::SpecialAmmoCrate))
			continue;
		if ((!Configs.Supply.ShowCompactAmmo &&
			!Configs.Supply.ShowMediumAmmo &&
			!Configs.Supply.ShowLongAmmo &&
			!Configs.Supply.ShowShortgunAmmo) && (type == EntityType::AmmoCrate))
			continue;
		if (!Configs.Supply.ShowMedkit && (type == EntityType::Medkit))
			continue;
		if (!Configs.Supply.ShowSupplyBox && (type == EntityType::SupplyBox))
			continue;

		int distance = (int)Vector3::Distance(ent->GetPosition(), CameraInstance->GetPosition());
		if (distance <= 0 || distance > Configs.Supply.MaxDistance)
			continue;

		if (!ent->GetValid())
			continue;
		Vector2 pos = CameraInstance->WorldToScreen(ent->GetPosition());
		if (pos.x == 0 || pos.y == 0)
			continue;

		std::string name = Configs.Supply.Name ? LOC("entity", ent->GetTypeAsString()) : "";
		std::string distanceText = Configs.Supply.Distance ? "[" + std::to_string(distance) + "m]" : "";

		ESPRenderer::DrawText(
			ImVec2(pos.x, pos.y),
			name + distanceText,
			Configs.Supply.TextColor,
			Configs.Supply.FontSize,
			Center
		);
	}
}

void DrawBloodBonds()
{
	std::vector<std::shared_ptr<WorldEntity>> templist = EnvironmentInstance->GetBloodBondsList();

	if (templist.size() == 0)
		return;

	for (std::shared_ptr<WorldEntity> ent : templist)
	{
		if (ent == nullptr)
			continue;

		int distance = (int)Vector3::Distance(ent->GetPosition(), CameraInstance->GetPosition());
		if (distance <= 0 || distance > Configs.BloodBonds.MaxDistance)
			continue;

		if (!ent->GetValid())
			continue;
		Vector2 pos = CameraInstance->WorldToScreen(ent->GetPosition());
		if (pos.x == 0 || pos.y == 0)
			continue;

		std::string name = Configs.BloodBonds.Name ? LOC("entity", ent->GetTypeAsString()) : "";
		std::string distanceText = Configs.BloodBonds.Distance ? "[" + std::to_string(distance) + "m]" : "";

		ESPRenderer::DrawText(
			ImVec2(pos.x, pos.y),
			name + distanceText,
			Configs.BloodBonds.TextColor,
			Configs.BloodBonds.FontSize,
			Center
		);
	}
}

void DrawTraps()
{
	std::vector<std::shared_ptr<WorldEntity>> templist = EnvironmentInstance->GetTrapList();

	if (templist.size() == 0)
		return;

	for (std::shared_ptr<WorldEntity> ent : templist)
	{
		if (ent == nullptr)
			continue;

		auto type = ent->GetType();
		if (!Configs.Trap.ShowBeartrap && (type == EntityType::BearTrap))
			continue;
		if (!Configs.Trap.ShowTripmines && (type == EntityType::TripMine))
			continue;
		if (!Configs.Trap.ShowDarksightDynamite && (type == EntityType::DarksightDynamite))
			continue;
		if (!Configs.Trap.ShowGunpowderBurrels && (type == EntityType::GunpowderBarrel))
			continue;
		if (!Configs.Trap.ShowOilBurrels && (type == EntityType::OilBarrel))
			continue;
		if (!Configs.Trap.ShowBioBurrels && (type == EntityType::BioBarrel))
			continue;

		int distance = (int)Vector3::Distance(ent->GetPosition(), CameraInstance->GetPosition());
		if (distance <= 0 || distance > Configs.Trap.MaxDistance)
			continue;

		if (!ent->GetValid())
			continue;
		Vector2 pos = CameraInstance->WorldToScreen(ent->GetPosition());
		if (pos.x == 0 || pos.y == 0)
			continue;

		std::string name = Configs.Trap.Name ? LOC("entity", ent->GetTypeAsString()) : "";
		std::string distanceText = Configs.Trap.Distance ? "[" + std::to_string(distance) + "m]" : "";

		ESPRenderer::DrawText(
			ImVec2(pos.x, pos.y),
			name + distanceText,
			(type == EntityType::BearTrap || type == EntityType::TripMine || type == EntityType::DarksightDynamite) ? Configs.Trap.TrapColor : Configs.Trap.BarrelColor,
			Configs.Trap.FontSize,
			Center
		);
	}
}

void DrawPOI()
{
	std::vector<std::shared_ptr<WorldEntity>> templist = EnvironmentInstance->GetPOIList();

	if (templist.size() == 0)
		return;

	for (std::shared_ptr<WorldEntity> ent : templist)
	{
		if (ent == nullptr)
			continue;

		auto type = ent->GetType();
		if (!Configs.POI.ShowResupplyStation && (type == EntityType::ResupplyStation))
			continue;
		if (!Configs.POI.ShowExtraction && (type == EntityType::ExtractionPoint))
			continue;
		if (!Configs.POI.ShowCashRegisters && (type == EntityType::CashRegister))
			continue;
		if (!Configs.POI.ShowPouches && (type == EntityType::Pouch))
			continue;
		if (!Configs.POI.ShowPosters && (type == EntityType::Poster))
			continue;
		if (!Configs.POI.ShowBlueprints && (type == EntityType::WorkbenchUpgrade))
			continue;
		if (!Configs.POI.ShowGunOil && (type == EntityType::WorkbenchOil))
			continue;
		if (!Configs.POI.ShowClues && (type == EntityType::Clue))
			continue;
		if (!Configs.POI.ShowSeasonalDestructibles && (type == EntityType::Event))
			continue;

		int distance = (int)Vector3::Distance(ent->GetPosition(), CameraInstance->GetPosition());
		if (distance <= 0 || distance > Configs.POI.MaxDistance)
			continue;

		if (!ent->GetValid())
			continue;

		Vector2 pos = CameraInstance->WorldToScreen(ent->GetPosition());
		if (pos.x == 0 || pos.y == 0)
			continue;

		std::string name = Configs.POI.Name ? LOC("entity", ent->GetTypeAsString()) : "";
		std::string distanceText = Configs.POI.Distance ? "[" + std::to_string(distance) + "m]" : "";

		ESPRenderer::DrawText(
			ImVec2(pos.x, pos.y),
			name + distanceText,
			Configs.POI.TextColor,
			Configs.POI.FontSize,
			Center
		);
	}
}

void DrawTraits()
{
	std::vector<std::shared_ptr<WorldEntity>> templist = EnvironmentInstance->GetTraitList();

	if (templist.size() == 0)
		return;

	for (std::shared_ptr<WorldEntity> ent : templist)
	{
		if (ent == nullptr)
			continue;

		auto type = ent->GetType();

		int distance = (int)Vector3::Distance(ent->GetPosition(), CameraInstance->GetPosition());
		if (distance <= 0 || distance > Configs.POI.MaxDistance)
			continue;

		if (!ent->GetValid())
			continue;

		Vector2 pos = CameraInstance->WorldToScreen(ent->GetPosition());
		if (pos.x == 0 || pos.y == 0)
			continue;

		std::string name = Configs.POI.Name ? LOC("entity", ent->GetTypeAsString()) + " [" + LOC("trait", ent->CompactTypeName) + "]" : "";
		std::string distanceText = Configs.POI.Distance ? "[" + std::to_string(distance) + "m]" : "";

		ESPRenderer::DrawText(
			ImVec2(pos.x, pos.y),
			name + distanceText,
			Configs.POI.TextColor,
			Configs.POI.FontSize,
			Center
		);
	}
}

void DrawOtherEsp()
{
	if (EnvironmentInstance == nullptr)
		return;

	if (EnvironmentInstance->GetObjectCount() < 10)
		return;

	if (Configs.Supply.Enable)
		DrawSupply();

	if (Configs.BloodBonds.Enable)
		DrawBloodBonds();

	if (Configs.Trap.Enable)
		DrawTraps();

	if (Configs.POI.Enable)
		DrawPOI();

	if (Configs.POI.ShowTraits)
		DrawTraits();
}