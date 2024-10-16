#pragma once
#include "WorldEntity.h"
class Environment
{
private:
	uint64_t SystemGlobalEnvironment = 0x0;
	uint64_t SystemGlobalEnvironmentOffset = 0x25D22E8; // This is the only thing that should need updating https://www.unknowncheats.me/forum/other-fps-games/350352-hunt-showdown-27.html someone would post it here.
	void FindSystemGlobalEnvironment();

	uint64_t EntitySystem = 0x0;
	uint64_t EntitySystemOffset = 0xC0;

	uint64_t pSystem = 0x0;
	uint64_t pSystemOffset = 0x90;

	uint16_t ObjectCount = 0x0;
	uint64_t ObjectCountOffset = 0x40092;
	
	uint64_t EntityList = 0x0;
	uint64_t EntityListOffset = 0x40098;

	int SpectatorCount = 0;

	std::vector<std::shared_ptr<WorldEntity>> PlayerList;
	std::vector<std::shared_ptr<WorldEntity>> BossesList;
	std::vector<std::shared_ptr<WorldEntity>> SupplyList;
	std::vector<std::shared_ptr<WorldEntity>> BloodBondsList;
	std::vector<std::shared_ptr<WorldEntity>> TrapList;
	std::vector<std::shared_ptr<WorldEntity>> POIList;

	bool createEntitiesDump = false;
public:
	uint64_t GetSystemGlobalEnvironment() { return SystemGlobalEnvironment; }
	uint64_t GetEntitySystem() { return EntitySystem; }
	uint64_t GetpSystem() { return pSystem; }
	uint16_t GetObjectCount() { return ObjectCount; }
	uint64_t GetEntityList() { return EntityList; }
	void GetEntities();
	void UpdatePlayerList();
	void UpdateBossesList();
	void CacheEntities();
	Environment();
	void ClearConsole();
	std::mutex PlayerListMutex;
	std::mutex BossesListMutex;
	std::mutex SupplyListMutex;
	std::mutex BloodBondsListMutex;
	std::mutex TrapListMutex;
	std::mutex POIListMutex;
	std::mutex SpectatorCountMutex;
	std::vector<std::shared_ptr<WorldEntity>> GetPlayerList() { return PlayerList; }
	std::vector<std::shared_ptr<WorldEntity>> GetBossesList() { return BossesList; }
	std::vector<std::shared_ptr<WorldEntity>> GetSupplyList() { return SupplyList; }
	std::vector<std::shared_ptr<WorldEntity>> GetBloodBondsList() { return BloodBondsList; }
	std::vector<std::shared_ptr<WorldEntity>> GetTrapList() { return TrapList; }
	std::vector<std::shared_ptr<WorldEntity>> GetPOIList() { return POIList; }
	int GetSpectatorCount() { return SpectatorCount; }
};