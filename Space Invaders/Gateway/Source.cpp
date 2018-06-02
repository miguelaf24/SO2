#include "../Space Invaders/utils.h"

#define PIPE_NAME TEXT("\\\\.\\pipe\\GamePipe")

BOOL(*OpenGame)(void);
BOOL(*OpenBuff)(void);
Command(*GetMSG)();
BOOL(*wrtMSG)(Command);
pBuff(*rbuff)();
Jogo(*getGame)();

HMODULE hDLL;
HANDLE eGameAcess, hGameUpdateThread, hPlayer[5], hConnectThread;
//THREADs
DWORD WINAPI GameUpdateThread(LPVOID params);
DWORD WINAPI thread_read(LPVOID data);
DWORD WINAPI connect_Thread(LPVOID data);

int _tmain(int argc, LPTSTR argv[]) {
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	srand((int)time(NULL));

	//DLL LOAD
	hDLL = LoadLibrary(_T("SpaceDLL"));

	if (hDLL == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> Loading DLL\n"));
		return 0;
	}

	//DLL functions start
	OpenGame = (BOOL(*)())GetProcAddress(hDLL, "openGameMemory");
	OpenBuff = (BOOL(*)())GetProcAddress(hDLL, "openBuffer");
	getGame = (Jogo(*)())GetProcAddress(hDLL, "getGame");
	rbuff = (pBuff(*)())GetProcAddress(hDLL, "returnBuff");

	OpenGame(); //carrega jogo da memoria
	OpenBuff(); //carrega buffer da memoria

	hConnectThread = CreateThread(NULL, 0, connect_Thread, NULL, 0, 0);
		if (hConnectThread == NULL) {
			_tprintf(TEXT("[(DEBUG)Thread:Erro-> Error starting game update thread\n %d  \n"), GetLastError());
			return 0;
		}

	hGameUpdateThread = CreateThread(NULL, 0, GameUpdateThread, NULL, 0, 0);
	if (hGameUpdateThread == NULL) {
		_tprintf(TEXT("[(DEBUG)Thread:Erro-> Error starting game update thread\n %d  \n"), GetLastError());
		return 0;
	}





	WaitForSingleObject(hGameUpdateThread, INFINITE);
	return 0;
}

void broadcast(Jogo gamedata) {
	OVERLAPPED Ov;
	HANDLE IOReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	BOOL ret;
	DWORD n;
	message msg;
	msg.jogo = gamedata;
	for (int i = 0; i < 5; i++) {
		if (hPlayer[i] != 0) {
			msg.idPlayer = i + 1;
			ZeroMemory(&Ov, sizeof(Ov));
			ResetEvent(IOReady);
			Ov.hEvent = IOReady;

			ret = WriteFile(hPlayer[i], &msg, MSGSIZE, &n, &Ov);

			WaitForSingleObject(IOReady, INFINITE);

			GetOverlappedResult(hPlayer[i], &Ov, &n, FALSE);

			if (n < MSGSIZE) {
				_tprintf(TEXT("[ERRO] Nao concluiu escrita! (WriteFile)\n"));
			}
			else {
				_tprintf(TEXT("[GATEWAY] Enviei %d bytes ao leitor...(WriteFile)\n"), n);
			}
		}
	}

}

DWORD WINAPI GameUpdateThread(LPVOID params) {
	eGameAcess = OpenEvent(EVENT_ALL_ACCESS, TRUE, _T("GameUpdateEvent"));
	Jogo gamedata;

	do {
		WaitForSingleObject(eGameAcess, INFINITE);
		_tprintf(TEXT("GameUpdateThread\n"));

		gamedata = getGame();

		broadcast(gamedata);//envia actualização a todos os players

		_tprintf(TEXT("Read Gamedata, navenormalX= %d\n"), gamedata.navesnormais[14].e.x);
		_tprintf(TEXT("Read Gamedata, navenormalY= %d\n"), gamedata.navesnormais[14].e.y);



	} while (1);

	_tprintf(TEXT("FIM GameUpdateThread\n"));
	return 0;


}

DWORD WINAPI connect_Thread(LPVOID data) {
	//THREAD T1
	int i;
	HANDLE p, hT;

	for (int i = 0; i < 5; i++)
		hPlayer[i] = NULL;


	while (1) {
		_tprintf(TEXT("[DEBUG] Criar uma cópia do pipe '%s' ... (CreateNamedPipe)\n"), PIPE_NAME);
		p = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, PIPE_UNLIMITED_INSTANCES, PIPEBUFFSIZE, PIPEBUFFSIZE, 1000, NULL);
		if (p == INVALID_HANDLE_VALUE) { _tprintf(TEXT("[ERRO] criar pipe! (CreateNamedPipe\n")); exit(-1); }


		_tprintf(TEXT("[DEBUG] Esperar ligação de um cliente...(ConnectNamedPipe)\n"));
		if (!ConnectNamedPipe(p, NULL)) {
			_tprintf(TEXT("[ERRO] Ligação ao cliente! (ConnectNamedPipe\n"));
			exit(-1);
		}

		for (i = 0; i < 5; i++) {
			if (hPlayer[i] == NULL) {
				hPlayer[i] = p;
				_tprintf(TEXT("[CONNECTED] Ligação ao cliente! (ConnectNamedPipe) \n"));
				break;
			}

		}

		//thread para tratar pedidos do pipe
		hT = CreateThread(NULL, 0, thread_read, (LPVOID)p, 0, 0);


	}
	//FIM THREAD
	return 0;
}

DWORD WINAPI thread_read(LPVOID data) {
	Command c;
	DWORD n;
	BOOL ret;
	HANDLE hPipeL = (HANDLE)data;
	HANDLE IOReady;
	wrtMSG = (BOOL(*)(Command))GetProcAddress(hDLL, "WriteBuffer");


	OVERLAPPED Ov;
	IOReady = CreateEvent(NULL, TRUE, FALSE, NULL);

	while (1) {
		ZeroMemory(&Ov, sizeof(Ov));
		ResetEvent(IOReady);
		Ov.hEvent = IOReady;

		ReadFile(hPipeL, &c, sizeof(Command), &n, &Ov);
		WaitForSingleObject(IOReady, INFINITE);
		ret = GetOverlappedResult(hPipeL, &Ov, &n, FALSE);

		if (!ret || !n) {
			_tprintf(TEXT("[ERRO] %d %d... (ReadFile)\n"), ret, n);
			break;
		}
		else {
			if (!wrtMSG(c)) {
				_tprintf(TEXT("Erro: não foi possível escrever do buffer -> %d\n"), GetLastError());
				return 0;
			}
		}
	}

	return 0;
}