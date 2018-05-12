#include "../Space Invaders/utils.h"

BOOL(*OpenGame)(void);
BOOL(*OpenBuff)(void);
Jogada(*GetMSG)();
BOOL(*wrtMSG)(Jogada);
pBuff(*rbuff)();

int _tmain(int argc, LPTSTR argv[]) {
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	srand((int)time(NULL));

	//DLL LOAD
	HMODULE hDLL = LoadLibrary("SpaceDLL");

	if (hDLL == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> Loading DLL\n"));
		return 0;
	}

	//DLL functions start
	OpenGame = (BOOL(*)())GetProcAddress(hDLL, "openGameMemory");
	OpenBuff = (BOOL(*)())GetProcAddress(hDLL, "openBuffer");
	GetMSG = (Jogada(*)())GetProcAddress(hDLL, "ReadBuffer");
	wrtMSG = (BOOL(*)(Jogada))GetProcAddress(hDLL, "WriteBuffer");
	rbuff = (pBuff(*)())GetProcAddress(hDLL, "returnBuff");

	OpenGame(); //carrega jogo da memoria
	OpenBuff(); //carrega buffer da memoria
	Jogada jogada, jogada1; jogada.id = 20; jogada.Dir = 20; jogada1.id = 71; jogada1.Dir = 71;

	
	if (!wrtMSG(jogada)) {
		_tprintf(TEXT("Erro: não foi possível escrever do buffer -> %d\n"), GetLastError());
		return 0;
	}
	
	if (!wrtMSG(jogada1)) {
		_tprintf(TEXT("Erro: não foi possível escrever do buffer -> %d\n"), GetLastError());
		return 0;
	}


}