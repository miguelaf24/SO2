#include "../Space Invaders/utils.h"


#define PIPE_NAME TEXT("\\\\.\\pipe\\GamePipe")

//VARIAVEIS GLOBAIS
HANDLE hPipe;

//THREADS
DWORD WINAPI thread_write(LPVOID data);
DWORD WINAPI thread_read(LPVOID data);


//MAIN
int _tmain(int argc, LPTSTR argv[]) {

	int i = 0;


#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	_tprintf(TEXT("[DEBUG] Esperar pelo pipe '%s' (WaitNamedPipe)\n"), PIPE_NAME);
	if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'!\n"), PIPE_NAME);
		Sleep(2000);
		exit(-1);
	}
	_tprintf(TEXT("[DEBUG] Ligação ao pipe do escritor... (CreateFile)\n"));
	hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0 | FILE_FLAG_OVERLAPPED, NULL);
	if (hPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), PIPE_NAME);
		Sleep(2000);
		exit(-1);
	}
	_tprintf(TEXT("[DEBUG] Liguei-me...\n"));


	//ALTERAR MODO DO PIPE
	DWORD modo = PIPE_READMODE_MESSAGE;
	SetNamedPipeHandleState(hPipe, &modo, NULL, NULL);

	HANDLE htTreadw;
	htTreadw = CreateThread(NULL, 0, thread_write, NULL, 0, NULL);
	HANDLE htTreadr;
	htTreadr = CreateThread(NULL, 0, thread_read, NULL, 0, NULL);


	WaitForSingleObject(htTreadr, INFINITE);
	WaitForSingleObject(htTreadw, INFINITE);
	CloseHandle(htTreadw);
	CloseHandle(htTreadr);
	CloseHandle(hPipe);
	Sleep(200);
	return 0;
}


DWORD WINAPI thread_read(LPVOID data) {
	message msg;
	BOOL ret=false;
	DWORD n;
	HANDLE IOReady;
	OVERLAPPED Ov;
	IOReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	while (1) {
		ZeroMemory(&Ov, sizeof(Ov));
		ResetEvent(IOReady);
		Ov.hEvent = IOReady;

		ret = ReadFile(hPipe, &msg, MSGSIZE, &n, &Ov);
		WaitForSingleObject(IOReady, INFINITE);
		 GetOverlappedResult(hPipe, &Ov, &n, FALSE);

		
	/*	if (!ret) {
			_tprintf(TEXT("[ERROR] READING %d %d... threadRead \n"), ret, n);
			continue;
		}*/
		if (n<MSGSIZE) {
			_tprintf(TEXT("[ERROR] Less bites received %d %d... (ReadFile)\n"), ret, n);
			//break;
		}
		else {
			_tprintf(TEXT("[DEBUG] Recebi %d bytes, jogodif: %d\n"), n, msg.jogo.dificuldade);
		}
	}
	return 0;
}

DWORD WINAPI thread_write(LPVOID data) {
	DWORD n;
	TCHAR buf[256];
	BOOL ret;
	//overlaped I/O
	HANDLE IOReady;
	OVERLAPPED Ov;
	Command c;
	IOReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	do {

		_tprintf(TEXT("COMMAND: "));
		_tprintf(TEXT("Insira id comando: "));
		_tscanf_s(TEXT("%d"), &c.id);
		_tprintf(TEXT("\nInsira valor do comando: "));
		_tscanf_s(TEXT("%d"), &c.cmd);


		ZeroMemory(&Ov, sizeof(Ov));
		ResetEvent(IOReady);
		Ov.hEvent = IOReady;

		WriteFile(hPipe, &c, sizeof(Command), &n, &Ov);

		WaitForSingleObject(IOReady, INFINITE);
		ret = GetOverlappedResult(hPipe, &Ov, &n, FALSE);

		if (!ret) {
			_tprintf(TEXT("[ERRO] Escrever no pipe!\n"));
			exit(-1);
		}
		_tprintf(TEXT("[DEBUG] Enviei %d bytes ao gateway...\n"), n);
	} while (_tcscmp(buf, TEXT("fim")));



	return 0;
}