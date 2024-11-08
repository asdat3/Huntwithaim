#pragma once
#include "WorldEntity.h"
class Environment
{
private:
	uint64_t SystemGlobalEnvironment = 0x0;
	const uint64_t SystemGlobalEnvironmentOffset = 0x0; // You can manually overwrite offset, if signature find is not working https://www.unknowncheats.me/forum/other-fps-games/350352-hunt-showdown-27.html someone would post it here.
	const char* SystemGlobalEnvironmentSignature = "48 8B 05 ? ? ? ? 48 8B 88 B0";
	void FindSystemGlobalEnvironment();

	uint64_t EntitySystem = 0x0;
	const uint64_t EntitySystemOffset = 0xC0;

	uint64_t pSystem = 0x0;
	const uint64_t pSystemOffset = 0x90;

	uint16_t ObjectCount = 0x0;
	const uint64_t ObjectCountOffset = 0x40092;
	
	uint64_t EntityList = 0x0;
	const uint64_t EntityListOffset = 0x40098;

	const char* LocalPlayerSignature = "48 8B 15 ? ? ? ? 48 2B F2 48 C1 FE 05 48 85 F6";
	uint64_t LocalPlayer = 0x0;

	int SpectatorCount = 0;

	std::vector<std::shared_ptr<WorldEntity>> PlayerList;
	std::vector<std::shared_ptr<WorldEntity>> BossesList;
	std::vector<std::shared_ptr<WorldEntity>> SupplyList;
	std::vector<std::shared_ptr<WorldEntity>> BloodBondsList;
	std::vector<std::shared_ptr<WorldEntity>> TrapList;
	std::vector<std::shared_ptr<WorldEntity>> POIList;
public:
	uint64_t GetSystemGlobalEnvironment() { return SystemGlobalEnvironment; }
	uint64_t GetEntitySystem() { return EntitySystem; }
	uint64_t GetpSystem() { return pSystem; }
	uint16_t GetObjectCount() { return ObjectCount; }
	uint64_t GetEntityList() { return EntityList; }
	uint64_t GetLocalPlayerPointer() { return LocalPlayer; }
	void UpdateLocalPlayer();
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
extern bool createEntitiesDump;