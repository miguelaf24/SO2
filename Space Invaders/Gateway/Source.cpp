#include "../Space Invaders/utils.h"

BOOL(*OpenGame)(void);
BOOL(*OpenBuff)(void);
Command(*GetMSG)();
BOOL(*wrtMSG)(Command);
pBuff(*rbuff)();
pJogo(*getGame)();

HANDLE eGameAcess, hGameUpdateThread;
//THREADs
DWORD WINAPI GameUpdateThread(LPVOID params);

int _tmain(int argc, LPTSTR argv[]) {
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	srand((int)time(NULL));

	//DLL LOAD
	HMODULE hDLL = LoadLibrary(_T("SpaceDLL"));

	if (hDLL == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> Loading DLL\n"));
		return 0;
	}

	//DLL functions start
	OpenGame = (BOOL(*)())GetProcAddress(hDLL, "openGameMemory");
	OpenBuff = (BOOL(*)())GetProcAddress(hDLL, "openBuffer");
	getGame= (pJogo(*)())GetProcAddress(hDLL, "getGame");
	wrtMSG = (BOOL(*)(Command))GetProcAddress(hDLL, "WriteBuffer");
	rbuff = (pBuff(*)())GetProcAddress(hDLL, "returnBuff");

	OpenGame(); //carrega jogo da memoria
	OpenBuff(); //carrega buffer da memoria

	hGameUpdateThread = CreateThread(NULL, 0, GameUpdateThread, NULL, 0, 0);
	if (hGameUpdateThread == NULL) {
		_tprintf(TEXT("[(DEBUG)Thread:Erro-> Error starting game update thread\n %d  \n"), GetLastError());
		return 0;
	}


	Command cmd;

	_tprintf(TEXT("TESTE COMANDOS GATEWAY \n"));
	do {
		_tprintf(TEXT("Insira id comando: "));
		_tscanf_s(TEXT("%d"), &cmd.id);
		_tprintf(TEXT("\nInsira valor do comando: "));
		_tscanf_s(TEXT("%d"), &cmd.cmd);

		if (!wrtMSG(cmd)) {
			_tprintf(TEXT("Erro: n�o foi poss�vel escrever do buffer -> %d\n"), GetLastError());
			return 0;
		}
		
	} while (1);
	_tprintf(TEXT("FIM gat\n"));
	return 0;
}

DWORD WINAPI GameUpdateThread(LPVOID params) {
	eGameAcess = OpenEvent(EVENT_ALL_ACCESS, TRUE, _T("GameUpdateEvent"));
	pJogo gamedata=NULL;
	
	do{
		WaitForSingleObject(eGameAcess, INFINITE);
		_tprintf(TEXT("GameUpdateThread\n"));

		gamedata=getGame();
		
		//envia actualiza��o a todos os players
		_tprintf(TEXT("Read Gamedata, dif= %d\n"), gamedata->dificuldade);
		_tprintf(TEXT("Read Gamedata, nnavesnormais= %d\n"), gamedata->nNavesNormais);
		_tprintf(TEXT("Read Gamedata, nnavesesquivas= %d\n"), gamedata->nNavesEsquivas);
		_tprintf(TEXT("Read Gamedata, navenormalX= %d\n"), gamedata->navesnormais[0].e.x);
	

		
	} while (1);

	_tprintf(TEXT("FIM GameUpdateThread\n"));
	return 0;

	
}