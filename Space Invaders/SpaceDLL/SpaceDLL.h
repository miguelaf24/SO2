#include <windows.h>
#include <tchar.h>
#include "../Space Invaders/utils.h"

HANDLE hGame, mGame;
Jogo pGameView


#define DLL_EXPORTS

#ifdef DLL_EXPORTS
#define DLL_SPACE __declspec(dllexport)
#else
#define DLL_SPACE __declspec(dllimport)
#endif

DLL_SPACE BOOL startSpaceServer(HANDLE);