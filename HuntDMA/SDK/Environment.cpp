#include "Pch.h"
#include "Environment.h"
#include "WorldEntity.h"
#include "Globals.h"
#include "ConfigUtilities.h"

bool createEntitiesDump = false;

Environment::Environment()
{
	auto handle = TargetProcess.CreateScatterHandle();
	if (SystemGlobalEnvironmentOffset == 0x0)
	{
		FindSystemGlobalEnvironment();
	}
	else
	{
		SystemGlobalEnvironment = TargetProcess.Read<uint64_t>(TargetProcess.GetBaseAddress("GameHunt.dll") + SystemGlobalEnvironmentOffset);
		LOG_DEBUG(LIT("SystemGlobalEnvironment: 0x%X"), SystemGlobalEnvironment);
	}
	TargetProcess.AddScatterReadRequest(handle, SystemGlobalEnvironment + EntitySystemOffset, &EntitySystem, sizeof(uint64_t));
	TargetProcess.AddScatterReadRequest(handle, SystemGlobalEnvironment + pSystemOffset, &pSystem, sizeof(uint64_t));
	TargetProcess.ExecuteReadScatter(handle);
	TargetProcess.CloseScatterHandle(handle);
	IGameFramework = TargetProcess.Read<uint64_t>(SystemGlobalEnvironment + IGameFrameworkOffset);
	pGame = TargetProcess.Read<uint64_t>(IGameFramework + pGameOffset);
	LOG_DEBUG(LIT("EntitySystem: 0x%X"), EntitySystem);
	LOG_DEBUG(LIT("pSystem: 0x%X"), pSystem);
}

void Environment::FindSystemGlobalEnvironment()
{
	auto base = TargetProcess.GetBaseAddress("GameHunt.dll");
	auto size = TargetProcess.GetBaseSize("GameHunt.dll");
	int relative;
	uint64_t SystemGlobEnvSig = TargetProcess.FindSignature(SystemGlobalEnvironmentSignature, base, base + size);
	LOG_DEBUG(LIT("SystemGlobEnvSig: 0x%X"), SystemGlobEnvSig);

	auto handle = TargetProcess.CreateScatterHandle();
	TargetProcess.AddScatterReadRequest(handle, SystemGlobEnvSig + 3, &relative, sizeof(int));
	TargetProcess.ExecuteReadScatter(handle);
	TargetProcess.AddScatterReadRequest(handle, SystemGlobEnvSig + 7 + relative, &SystemGlobalEnvironment, sizeof(SystemGlobalEnvironment));
	TargetProcess.ExecuteReadScatter(handle);
	TargetProcess.CloseScatterHandle(handle);
	LOG_DEBUG(LIT("SystemGlobalEnvironment: 0x%X"), SystemGlobalEnvironment);
}

void Environment::GetEntities()
{
	ObjectCount = TargetProcess.Read<uint16_t>(EntitySystem + ObjectCountOffset) + 1;
	LOG_INFO(LIT("ObjectCount: %d"), ObjectCount);
	EntityList = EntitySystem + EntityListOffset;
}

void Environment::UpdateLocalPlayer()
{
	pGameClientNub = TargetProcess.Read<uint64_t>(pGame + pGameClientNubOffset);
	pGameClientChannel = TargetProcess.Read<uint64_t>(pGameClientNub + pGameClientChannelOffset);
	localPlayerIdx = TargetProcess.Read<int>(pGameClientChannel + localPlayerIdxOffset);
	LocalPlayer = TargetProcess.Read<uint64_t>(EntityList + (localPlayerIdx * sizeof(uint64_t)));

	//LogTrait();
	//LOG_INFO("LocalPlayerIndex = %llu", localPlayerIdx);
}

void Environment::LogTrait()
{
	auto base = TargetProcess.GetBaseAddress("GameHunt.dll");
	std::vector<std::shared_ptr<WorldEntity>> tempPoiList = EnvironmentInstance->GetPOIList();
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

	for (size_t index = 0; index < tempPoiList.size(); ++index)
	{
		std::shared_ptr<WorldEntity> ent = tempPoiList[index];
		if (ent == nullptr)
			continue;
		if (ent->GetType() == EntityType::Trait)
		{
			LOG_INFO("%llx: [%f m], name=[%s], x=%f y=%f z=%f", ent->GetClass(), Vector3::Distance(ent->GetPosition(), LocalPlayer->GetPosition()), ent->GetTypeName(), ent->GetPosition().x, ent->GetPosition().y, ent->GetPosition().z);
		}
	}
}

void Environment::UpdatePlayerList()
{
	std::vector<std::shared_ptr<WorldEntity>> templist = EnvironmentInstance->GetPlayerList();
	if (templist.size() == 0)
		return;
	auto handle = TargetProcess.CreateScatterHandle();
	auto writehandle = TargetProcess.CreateScatterHandle();
	//for (std::shared_ptr<WorldEntity> ent : templist)
	for (size_t index = 0; index < templist.size(); ++index)
	{
		std::shared_ptr<WorldEntity> ent = templist[index];
		if (ent == nullptr)
			continue;
		if (ent->GetType() == EntityType::LocalPlayer)
		{
			TargetProcess.AddScatterReadRequest(handle, ent->SpecCountPointer4 + ent->SpecCountOffset5, &ent->SpecCount, sizeof(int));
			continue;
		}	
		if (!(ent->GetClass() > 0x2000000 && ent->GetClass() < 0x7FFFFFFFFFFF))
		{
			ent->SetValid(false); // check if the player is still alive/active
			continue;
		}
		if (Configs.Player.Chams && ent->GetType() != EntityType::FriendlyPlayer)
			ent->WriteNode(writehandle, Configs.Player.ChamMode, Configs.Player.Chams);
		ent->UpdateNode(handle);
		ent->UpdatePosition(handle);
		ent->UpdateHealth(handle);
		ent->UpdateClass(handle);
		ent->UpdateExtraction(handle);
	}
	Configs.Player.Chams = false;
	TargetProcess.ExecuteReadScatter(handle);
	TargetProcess.ExecuteWriteScatter(writehandle);

	TargetProcess.CloseScatterHandle(handle);
	TargetProcess.CloseScatterHandle(writehandle);

	bool spectatorCountChanged = false;
	for (size_t index = 0; index < templist.size(); ++index)
	{
		std::shared_ptr<WorldEntity> ent = templist[index];
		if (ent == nullptr)
			continue;
		if (ent->GetType() == EntityType::LocalPlayer)
		{
			spectatorCountChanged = true;
			SpectatorCount = ent->SpecCount;
			continue;
		}

		ent->SetHidden((ent->GetInternalFlags() & WorldEntity::HIDDEN_FLAG) == WorldEntity::HIDDEN_FLAG); // If player has extracted
		//ent->UpdateBones();
	}
	if (!spectatorCountChanged)
		SpectatorCount = 0;

	PlayerList = templist;
}

void Environment::UpdateBossesList()
{
	std::vector<std::shared_ptr<WorldEntity>> templist = EnvironmentInstance->GetBossesList();
	if (templist.size() == 0)
		return;
	auto handle = TargetProcess.CreateScatterHandle();
	auto writehandle = TargetProcess.CreateScatterHandle();
	for (std::shared_ptr<WorldEntity> ent : templist)
	{
		if (ent == nullptr)
			continue;
		if (!(ent->GetClass() > 0x2000000 && ent->GetClass() < 0x7FFFFFFFFFFF))
		{
			ent->SetValid(false); // TODO might be wrong, bosses happen to disappear from ESP
			continue;
		}
		ent->UpdateNode(handle);
		ent->UpdatePosition(handle);
		ent->UpdateClass(handle);
	}
	TargetProcess.ExecuteReadScatter(handle);
	TargetProcess.ExecuteWriteScatter(writehandle);

	TargetProcess.CloseScatterHandle(handle);
	TargetProcess.CloseScatterHandle(writehandle);

	BossesList = templist;
}

void Environment::CacheEntities()
{
	GetEntities();

	std::unique_ptr<uint64_t[]> object_raw_ptr = std::make_unique<uint64_t[]>(ObjectCount);
	TargetProcess.Read(EntityList, object_raw_ptr.get(), ObjectCount * sizeof(uint64_t));

	// Create entity pointers list
	std::vector<std::shared_ptr<WorldEntity>> entitypointerlist;
	entitypointerlist.resize(ObjectCount);

	for (int i = 0; i < ObjectCount; i++)
	{
		if (object_raw_ptr[i] == NULL)
			continue;
		
		entitypointerlist[i] = std::make_shared<WorldEntity>(object_raw_ptr[i]);
	}

	auto handle = TargetProcess.CreateScatterHandle();
	for (std::shared_ptr<WorldEntity> ent : entitypointerlist)
	{
		if (ent == nullptr)
			continue;
		ent->SetUp(handle);
	}
	TargetProcess.ExecuteReadScatter(handle);

	for (std::shared_ptr<WorldEntity> ent : entitypointerlist)
	{
		if (ent == nullptr)
			continue;
		ent->SetUp1(handle);
	}
	TargetProcess.ExecuteReadScatter(handle);

	for (std::shared_ptr<WorldEntity> ent : entitypointerlist)
	{
		if (ent == nullptr)
			continue;
		ent->SetUp2(handle);
	}
	TargetProcess.ExecuteReadScatter(handle);
	TargetProcess.CloseScatterHandle(handle);

	// doing this after we have read class names fully to avoid reading things we don't need

	std::list<std::string> entitiesDump;

	std::vector<std::shared_ptr<WorldEntity>> templayerlist;
	std::vector<std::shared_ptr<WorldEntity>> tempbosseslist;
	std::vector<std::shared_ptr<WorldEntity>> tempsupplylist;
	std::vector<std::shared_ptr<WorldEntity>> tempboodboundslist;
	std::vector<std::shared_ptr<WorldEntity>> temptraplist;
	std::vector<std::shared_ptr<WorldEntity>> temppoilist;
	for (std::shared_ptr<WorldEntity> ent : entitypointerlist)
	{
		if (ent == nullptr)
			continue;
		char* entityName = ent->GetEntityName().name;
		char* entityClassName = ent->GetEntityClassName().name;
		char* TraitName = ent->GetTypeName().name;

		if (createEntitiesDump)
			entitiesDump.push_back("Class: [" + std::string(entityClassName) + "]; Name: [" + std::string(entityName) + "], TypeName: [" + std::string(TraitName) + "];");
 
		if (strstr(entityClassName, "ObjectSpawner") != NULL) // We do not want spawners to show as objects
			continue;
		if (entityClassName == nullptr || entityClassName[0] == '\0')
			continue;
		if (strstr(entityClassName, "Hunter_Loot") != NULL)
		{
			ent->SetType(EntityType::DeadPlayer);
			templayerlist.push_back(ent);
			continue;
		}
		if (strstr(entityClassName, "HunterBasic") != NULL)
		{
			if (ent->IsLocalPlayer())
				ent->SetType(EntityType::LocalPlayer);
			else
				ent->SetType(EntityType::EnemyPlayer);
			templayerlist.push_back(ent);
			continue;
		}
		if (strstr(entityClassName, "target_assassin") != NULL)
		{
			ent->SetType(EntityType::Assassin);
			tempbosseslist.push_back(ent);
			continue;
		}
		if (strstr(entityClassName, "target_butcher") != NULL)
		{
			ent->SetType(EntityType::Butcher);
			tempbosseslist.push_back(ent);
			continue;
		}
		if (strstr(entityClassName, "target_spider") != NULL)
		{
			ent->SetType(EntityType::Spider);
			tempbosseslist.push_back(ent);
			continue;
		}
		if (strstr(entityClassName, "target_scrapbeak") != NULL)
		{
			ent->SetType(EntityType::Scrapbeak);
			tempbosseslist.push_back(ent);
			continue;
		}
		if (strstr(entityClassName, "target_rotjaw") != NULL)
		{
			ent->SetType(EntityType::Rotjaw);
			tempbosseslist.push_back(ent);
			continue;
		}
		if (strstr(entityClassName, "immolator_elite") != NULL)
		{
			ent->SetType(EntityType::Hellborn);
			tempbosseslist.push_back(ent);
			continue;
		}
		if (strstr(entityClassName, "ExtractionPoint") || strstr(entityClassName, "ExtractionZone") != NULL)
		{
			ent->SetType(EntityType::ExtractionPoint);
			temppoilist.push_back(ent);
			continue;
		}
		if (strstr(entityClassName, "ResupplyStation") != NULL)
		{
			ent->SetType(EntityType::ResupplyStation);
			temppoilist.push_back(ent);
			continue;
		}
		if ((std::string)(entityClassName) == "AmmoSwapBox")
		{
			ent->SetType(EntityType::AmmoSwapBox);

			if (strstr(entityName, "bullets_spitzer") != NULL)
			{
				ent->SetType(EntityType::SpitzerBullets);
			}
			else if (strstr(entityName, "bullets_poison") != NULL)
			{
				ent->SetType(EntityType::PoisonBullets);
			}
			else if (strstr(entityName, "bullets_highvelocity") != NULL)
			{
				ent->SetType(EntityType::HighVelocityBullets);
			}
			else if (strstr(entityName, "bullets_incendiary") != NULL)
			{
				ent->SetType(EntityType::IncendiaryBullets);
			}
			else if (strstr(entityName, "bullets_dumdum") != NULL)
			{
				ent->SetType(EntityType::DumDumBullets);
			}
			else if (strstr(entityName, "bullets_explosive") != NULL)
			{
				ent->SetType(EntityType::ExplosiveBullets);
			}
			else if (strstr(entityName, "bullets_fmj") != NULL)
			{
				ent->SetType(EntityType::FMJBullets);
			}
			else if (strstr(entityName, "bolts_arrows_poison") != NULL)
			{
				ent->SetType(EntityType::PoisonBolts);
			}
			else if (strstr(entityName, "bolts_arrows_explosive") != NULL)
			{
				ent->SetType(EntityType::ExplosiveBolts);
			}
			else if (strstr(entityName, "shells_dragonbreath") != NULL)
			{
				ent->SetType(EntityType::IncendiaryShells);
			}
			else if (strstr(entityName, "shells_pennyshot") != NULL)
			{
				ent->SetType(EntityType::PennyShotShells);
			}
			else if (strstr(entityName, "shells_flechette") != NULL)
			{
				ent->SetType(EntityType::FlechetteShells);
			}
			else if (strstr(entityName, "shells_slugs") != NULL)
			{
				ent->SetType(EntityType::SlugShells);
			}

			tempsupplylist.push_back(ent);
			continue;
		}
		if (strstr(entityName, "Oil_barrel") != NULL)
		{
			ent->SetType(EntityType::OilBarrel);
			temptraplist.push_back(ent);
			continue;
		}
		if (strstr(entityName, "Gunpowder_barrel") != NULL)
		{
			ent->SetType(EntityType::GunpowderBarrel);
			temptraplist.push_back(ent);
			continue;
		}
		if (strstr(entityName, "Poison_barrel") != NULL)
		{
			ent->SetType(EntityType::BioBarrel);
			temptraplist.push_back(ent);
			continue;
		}
		if ((std::string)(entityClassName) == "SmallLoot")
		{
			ent->SetType(EntityType::Unknown);

			if (strstr(entityName, "currency_collection") != NULL)
			{
				ent->SetType(EntityType::Pouch);
			}
			else if (strstr(entityName, "posters_collection") != NULL)
			{
				ent->SetType(EntityType::Poster);
			}
			else if (strstr(entityName, "workbench_upgrade_collection") != NULL)
			{
				ent->SetType(EntityType::WorkbenchUpgrade);
			}

			if (ent->GetType() == EntityType::Unknown)
			{
				LOG_ERROR(LIT("Unknown type: [%s]"), entityName);
				continue;
			}

			temppoilist.push_back(ent);
			continue;
		}
		if ((std::string)(entityClassName) == "SmallLootGunOil")
		{
			ent->SetType(EntityType::WorkbenchOil);
			temppoilist.push_back(ent);
			continue;
		}
		if ((std::string)(entityClassName) == "TraitCharm")
		{
			ent->SetType(EntityType::Trait);
			temppoilist.push_back(ent);
			continue;
		}
		if ((std::string)(entityClassName) == "Clue_Slave")
		{
			ent->SetType(EntityType::Clue);
			temppoilist.push_back(ent);
			continue;
		}
		if ((std::string)(entityClassName) == "Supply_Box")
		{
			ent->SetType(EntityType::Unknown);

			if (strstr(entityName, "cash_register_golden") != NULL)
			{
				ent->SetType(EntityType::GoldCashRegister);
				tempboodboundslist.push_back(ent);
				continue;
			}

			if (strstr(entityName, "cash_register") != NULL)
			{
				ent->SetType(EntityType::CashRegister);
				temppoilist.push_back(ent);
				continue;
			}

			if (strstr(entityName, "item_box") != NULL)
			{
				ent->SetType(EntityType::SupplyBox);
			}
			else if (strstr(entityName, "ammo_crate_normal") != NULL) // only in training mode
			{
				ent->SetType(EntityType::AmmoCrate);
			}
			else if (strstr(entityName, "ammo_crate_special") != NULL) // only in training mode
			{
				ent->SetType(EntityType::SpecialAmmoCrate);
			}
			else if (strstr(entityName, "ammo_box_special") != NULL)
			{
				ent->SetType(EntityType::SpecialAmmoCrate);
			}
			else if (strstr(entityName, "ammo_box") != NULL)
			{
				ent->SetType(EntityType::AmmoCrate);
			}
			else if (strstr(entityName, "ammo_package_medium") != NULL)
			{
				ent->SetType(EntityType::MediumAmmoPackage);
			}
			else if (strstr(entityName, "ammo_package_shotgun") != NULL)
			{
				ent->SetType(EntityType::ShotgunAmmoPackage);
			}
			else if (strstr(entityName, "ammo_package_compact") != NULL)
			{
				ent->SetType(EntityType::CompactAmmoPackage);
			}
			else if (strstr(entityName, "ammo_package_long") != NULL)
			{
				ent->SetType(EntityType::LongAmmoPackage);
			}
			else if (strstr(entityName, "health") != NULL || strstr(entityName, "Doctor") != NULL)
			{
				ent->SetType(EntityType::Medkit);
			}

			if (ent->GetType() == EntityType::Unknown) // for example all tower supplies have the same name, I skip them
				continue;

			tempsupplylist.push_back(ent);
			continue;
		}
		if ((std::string)(entityClassName) == "beartrap")
		{
			ent->SetType(EntityType::BearTrap);
			temptraplist.push_back(ent);
			continue;
		}
		if (((std::string)entityClassName).find("TripMine") != std::string::npos && ((std::string)entityClassName).find("2") == std::string::npos)
		{
			ent->SetType(EntityType::TripMine);
			temptraplist.push_back(ent);
			continue;
		}
		if ((std::string)(entityClassName) == "DarkSightDynamite")
		{
			ent->SetType(EntityType::DarksightDynamite);
			temptraplist.push_back(ent);
			continue;
		}
		if ((std::string)(entityClassName) == "DestroyableReward")
		{
			ent->SetType(EntityType::Event);
			temppoilist.push_back(ent);
			continue;
		}
	}

	handle = TargetProcess.CreateScatterHandle();
	for (std::shared_ptr<WorldEntity> ent : templayerlist)
	{
		if (ent == nullptr)
			continue;
		ent->SetUp3(handle);
	}
	for (std::shared_ptr<WorldEntity> ent : tempbosseslist)
	{
		if (ent == nullptr)
			continue;
		ent->SetUp3(handle);
	}
	for (std::shared_ptr<WorldEntity> ent : tempsupplylist)
	{
		if (ent == nullptr)
			continue;
		ent->SetUp3(handle);
	}
	for (std::shared_ptr<WorldEntity> ent : tempboodboundslist)
	{
		if (ent == nullptr)
			continue;
		ent->SetUp3(handle);
	}
	for (std::shared_ptr<WorldEntity> ent : temptraplist)
	{
		if (ent == nullptr)
			continue;
		ent->SetUp3(handle);
	}
	for (std::shared_ptr<WorldEntity> ent : temppoilist)
	{
		if (ent == nullptr)
			continue;
		ent->SetUp3(handle);
	}
	TargetProcess.ExecuteReadScatter(handle);

	if (createEntitiesDump)
	{
		std::ofstream outFile("classes-dump.txt", std::ios::trunc);

		if (outFile.is_open()) {
			for (const std::string& str : entitiesDump) {
				outFile << str << std::endl;
			}

			outFile.close();
			LOG_INFO(LIT("Rewrote classes dump."));
		}
		else {
			LOG_ERROR(LIT("Could not write entities dump!"));
		}
	}

	for (std::shared_ptr<WorldEntity> ent : templayerlist) // got to do this after set up 3
	{
		if (ent->GetRenderNode().silhouettes_param == 0x8CD2FF)
		{
			ent->SetType(EntityType::FriendlyPlayer);
		}
	}

	for (std::shared_ptr<WorldEntity> ent : templayerlist)
	{
		if (ent == nullptr)
			continue;
		ent->SetUp4(handle);
	}
	TargetProcess.ExecuteReadScatter(handle);
	TargetProcess.CloseScatterHandle(handle);

	std::swap(PlayerList, templayerlist);
	std::swap(BossesList, tempbosseslist);
	std::swap(SupplyList, tempsupplylist);
	std::swap(BloodBondsList, tempboodboundslist);
	std::swap(TrapList, temptraplist);
	std::swap(POIList, temppoilist);
}

void Environment::ClearConsole()
{
#if defined _WIN32
	system("cls");
	//clrscr(); // including header file : conio.h
#elif defined (__LINUX__) || defined(__gnu_linux__) || defined(__linux__)
	system("clear");
	//std::cout<< u8"\033[2J\033[1;1H"; //Using ANSI Escape Sequences 
#elif defined (__APPLE__)
	system("clear");
#endif
}
