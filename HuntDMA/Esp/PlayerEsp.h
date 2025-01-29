#pragma once
extern std::shared_ptr<CheatFunction> UpdatePlayers;
extern std::shared_ptr<CheatFunction> UpdateBosses;
extern void DrawPlayersEsp();
extern void DrawRadar();
extern void DrawBossesEsp();
extern bool IsValidHP(int hp);