
#pragma once  
#include <iostream>
#include <windows.h>
#include <tchar.h>
#include "stdafx.h"
#include "../Space Invaders/utils.h"


#define DLL_EXPORTS

#ifdef DLL_EXPORTS
#define DLL_SPACE __declspec(dllexport)
#else
#define DLL_SPACE __declspec(dllimport)
#endif


extern "C"
{
	DLL_SPACE BOOL openGameMemory();
	DLL_SPACE BOOL setGame(Jogo game);
	DLL_SPACE Jogo getGame();
	DLL_SPACE BOOL startBuffer();
	DLL_SPACE BOOL openBuffer();
	DLL_SPACE BOOL WriteBuffer(Command jogada);
	DLL_SPACE BOOL ReadBuffer(pCommand cmd);

}