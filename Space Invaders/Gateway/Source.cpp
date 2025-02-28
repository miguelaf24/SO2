#define _WIN32_WINNT 0x0500


#include "../Space Invaders/utils.h"
#include <sddl.h>
#define PIPE_NAME TEXT("\\\\.\\pipe\\GamePipe")

BOOL(*OpenGame)(void);
BOOL(*OpenBuff)(void);
Command(*GetMSG)();
BOOL(*wrtMSG)(Command);
pBuff(*rbuff)();
Jogo(*getGame)();
Jogo gamedata;
HMODULE hDLL;
HANDLE eGameAcess, hGameUpdateThread, hGameStartThread, hPlayer[5], hConnectThread, p;
HANDLE hT[5];
BOOL StartThreads(BOOL reset);
BOOL GetLogin(HANDLE hPipeL, int PlayerID);
//THREADs
DWORD WINAPI GameUpdateThread(LPVOID params);
DWORD WINAPI thread_read(LPVOID data);
DWORD WINAPI connect_Thread(LPVOID data);
DWORD WINAPI GameStart_Thread(LPVOID data);

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
	wrtMSG = (BOOL(*)(Command))GetProcAddress(hDLL, "WriteBuffer");

	OpenGame(); //carrega jogo da memoria
	OpenBuff(); //carrega buffer da memoria


	if (!StartThreads(FALSE))
		return 0;

	WaitForSingleObject(hGameUpdateThread, INFINITE);
	return 0;
}

BOOL StartThreads(BOOL reset) {

	if (reset) {
		Sleep(2000);
		TerminateThread(GameUpdateThread, 0);


		for (int i = 0; i < 5; i++) {
			CloseHandle(hPlayer[i]);
			if (hT[i] != NULL)
				TerminateThread(hT[i], 0);
			hT[i] = NULL;
			hPlayer[i] = NULL;
		}
		gamedata.gameover = FALSE;
		Sleep(1000);
	}
	else {
		hGameUpdateThread = CreateThread(NULL, 0, GameUpdateThread, NULL, 0, 0);
		if (hGameUpdateThread == NULL) {
			_tprintf(TEXT("[(DEBUG)Thread:Erro-> Error starting game update thread\n %d  \n"), GetLastError());
			return FALSE;
		}

	}

	hConnectThread = CreateThread(NULL, 0, connect_Thread, NULL, 0, 0);
	if (hConnectThread == NULL) {
		_tprintf(TEXT("[(DEBUG)Thread:Erro-> Error starting game update thread\n %d  \n"), GetLastError());
		return FALSE;
	}
	hGameStartThread = CreateThread(NULL, 0, GameStart_Thread, NULL, 0, 0);
	if (hGameStartThread == NULL) {
		_tprintf(TEXT("[(DEBUG)Thread:Erro-> Error starting game update thread\n %d  \n"), GetLastError());
		return FALSE;
	}

	return TRUE;
}

void broadcast(Jogo gamedata) {
	OVERLAPPED Ov;
	HANDLE IOReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	BOOL ret;
	DWORD nb;
	message msg;
	msg.jogo = gamedata;
	for (int i = 0; i < 5; i++) {
		if (hPlayer[i] != 0) {
			msg.idPlayer = i + 1;
			ZeroMemory(&Ov, sizeof(Ov));
			ResetEvent(IOReady);
			Ov.hEvent = IOReady;

			ret = WriteFile(hPlayer[i], &msg, MSGSIZE, &nb, &Ov);

			WaitForSingleObject(IOReady, INFINITE);

			GetOverlappedResult(hPlayer[i], &Ov, &nb, FALSE);

			if (nb < MSGSIZE) {
				_tprintf(TEXT("[ERRO] Nao concluiu escrita! (WriteFile)\n"));
			}

		}
	}

}

DWORD WINAPI GameUpdateThread(LPVOID params) {
	eGameAcess = OpenEvent(EVENT_ALL_ACCESS, TRUE, _T("GameUpdateEvent"));

	do {
		WaitForSingleObject(eGameAcess, INFINITE);

		gamedata = getGame();

		broadcast(gamedata);//envia actualiza��o a todos os players

		if (gamedata.gameover) {
			StartThreads(TRUE);
		}


	} while (1);

	_tprintf(TEXT("FIM GameUpdateThread\n"));
	return 0;


}

DWORD WINAPI GameStart_Thread(LPVOID data) {
	HANDLE eGameStart = OpenEvent(EVENT_ALL_ACCESS, TRUE, _T("GameStartEvent"));
	WaitForSingleObject(eGameStart, INFINITE);
	TerminateThread(hConnectThread, 0);
	DisconnectNamedPipe(p);
	CloseHandle(p);
	_tprintf(TEXT("Finalizei thread de connects\n"));

	return 0;
}
BOOL CreateMyDACL(SECURITY_ATTRIBUTES * pSA)
{
	// Define the SDDL for the DACL. This example sets 
	// the following access:
	//     Built-in guests are denied all access.
	//     Anonymous logon is denied all access.
	//     Authenticated users are allowed 
	//     read/write/execute access.
	//     Administrators are allowed full control.
	// Modify these values as needed to generate the proper
	// DACL for your application. 
	TCHAR * szSD = (_TCHAR*)TEXT("D:")       // Discretionary ACL
		TEXT("(D;OICI;GA;;;BG)")     // Deny access to 
									 // built-in guests
		TEXT("(A;OICI;GA;;;AN)")     // Deny access to 
									 // anonymous logon
		TEXT("(A;OICI;GRGWGX;;;AU)") // Allow 
									 // read/write/execute 
									 // to authenticated 
									 // users
		TEXT("(A;OICI;GA;;;BA)");    // Allow full control 
									 // to administrators

	if (NULL == pSA)
		return FALSE;

	return ConvertStringSecurityDescriptorToSecurityDescriptor(
		szSD,
		SDDL_REVISION_1,
		&(pSA->lpSecurityDescriptor),
		NULL);
}


DWORD WINAPI connect_Thread(LPVOID data) {
	//THREAD T1
	int i;
	SECURITY_ATTRIBUTES sa;


	if (!CreateMyDACL(&sa)) {
		// Error encountered; generate message and exit.
		printf("Failed CreateMyDACL\n");
		exit(1);
	}
	for (int i = 0; i < 5; i++)
		hPlayer[i] = NULL;


	while (1) {
		_tprintf(TEXT("[DEBUG] Criar uma c�pia do pipe '%s' ... (CreateNamedPipe)\n"), PIPE_NAME);
		p = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, PIPE_UNLIMITED_INSTANCES, PIPEBUFFSIZE, PIPEBUFFSIZE, 1000, &sa);
		if (p == INVALID_HANDLE_VALUE) {
			_tprintf(TEXT("[ERRO] criar pipe! (CreateNamedPipe\n")); exit(-1);
		}


		_tprintf(TEXT("[DEBUG] Esperar liga��o de um cliente...(ConnectNamedPipe)\n"));
		if (!ConnectNamedPipe(p, NULL)) {
			_tprintf(TEXT("[ERRO] Liga��o ao cliente! (ConnectNamedPipe\n"));
			exit(-1);
		}

		for (i = 0; i < 5; i++) {
			if (hPlayer[i] == NULL) {
				hPlayer[i] = p;
				_tprintf(TEXT("[CONNECTED] Liga��o ao cliente! (ConnectNamedPipe) \n"));
				hT[i] = CreateThread(NULL, 0, thread_read, (LPVOID)i, 0, 0);
				if (hT[i] == NULL)
				{
					_tprintf(TEXT("[DEBUG] ERRO AO INICIAR THREAD \n"));
				}
				break;
			}

		}
		//	GetLogin(p, i);
		//thread para tratar pedidos do pipe



	}
	//FIM THREAD
	return 0;
}

BOOL GetLogin(HANDLE hPipeL, int PlayerID) {
	HANDLE IOReady;
	OVERLAPPED Ov;
	IOReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	DWORD nl;
	BOOL ret;
	char username[20];

	ZeroMemory(&Ov, sizeof(Ov));
	ResetEvent(IOReady);
	Ov.hEvent = IOReady;

	ReadFile(hPipeL, &username, sizeof(char) * 20, &nl, &Ov);
	WaitForSingleObject(IOReady, INFINITE);
	ret = GetOverlappedResult(hPipeL, &Ov, &nl, FALSE);

	if (!ret || !nl) {
		_tprintf(TEXT("[ERRO](ao adquirir login de user)\n"));
	}
	_tprintf(TEXT("NOME: %hs\n"), username);


	//Envia o ID ao player
	DWORD nll;

	ZeroMemory(&Ov, sizeof(Ov));
	ResetEvent(IOReady);
	Ov.hEvent = IOReady;

	ret = WriteFile(hPipeL, &PlayerID, sizeof(int), &nll, &Ov);

	WaitForSingleObject(IOReady, INFINITE);

	GetOverlappedResult(hPipeL, &Ov, &nll, FALSE);

	if (nll < sizeof(int)) {
		_tprintf(TEXT("[ERRO] Nao concluiu escrita! (WriteFile)\n"));
		return FALSE;
	}

	wrtMSG = (BOOL(*)(Command))GetProcAddress(hDLL, "WriteBuffer");
	Command c;
	strcpy_s(c.username, username);
	c.id = PlayerID;
	c.cmd = 0;
	wrtMSG(c);
	return TRUE;
}


DWORD WINAPI thread_read(LPVOID data) {
	Command c;
	DWORD rn;
	BOOL ret;
	int PlayerID = (int)data;
	HANDLE hPipeL = hPlayer[PlayerID];
	HANDLE IOReady;
	//wrtMSG = (BOOL(*)(Command))GetProcAddress(hDLL, "WriteBuffer");
	_tprintf(TEXT("GET LOGIN\n"));
	GetLogin(hPipeL, PlayerID);

	OVERLAPPED Ov;
	IOReady = CreateEvent(NULL, TRUE, FALSE, NULL);

	while (1) {
		ZeroMemory(&Ov, sizeof(Ov));
		ResetEvent(IOReady);
		Ov.hEvent = IOReady;

		ReadFile(hPipeL, &c, sizeof(Command), &rn, &Ov);
		WaitForSingleObject(IOReady, INFINITE);
		ret = GetOverlappedResult(hPipeL, &Ov, &rn, FALSE);

		if (!ret || !rn) {
			_tprintf(TEXT("[ERRO] %d %d... (ReadFile)\n"), ret, rn);
			break;
		}
		else {

			if (!wrtMSG(c)) {
				_tprintf(TEXT("Erro: n�o foi poss�vel escrever do buffer -> %d\n"), GetLastError());
				return 0;
			}
		}
	}


	return 0;
}