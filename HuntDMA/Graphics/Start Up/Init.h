#pragma once
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "Dwrite")
#pragma comment(lib, "windowscodecs.lib")

const bool enableAimBot = false;

extern std::shared_ptr<CheatFunction> Cache;
extern std::shared_ptr<CheatFunction> UpdateCam;
extern std::shared_ptr<CheatFunction> UpdateLocalPlayer;

void InitialiseClasses();
void CacheThread();
void InitializeESP();

extern int FrameRate();