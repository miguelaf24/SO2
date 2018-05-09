#include "utils.h"

BOOL(*StartServer)(void);
BOOL(*StartBuff)(void);
Jogada(*GetMSG)();
BOOL(*wrtMSG)(Jogada);
BOOL(*setbuff)(pBuff);

int _tmain(int argc, LPTSTR argv[]) {
	TCHAR resp;
	int y;
	

	//DWORD threadId[]; //Id da thread a ser criada
	//HANDLE hT[4] = (HANDLE *)malloc(4 * sizeof(HANDLE)); //HANDLE/ponteiro para a thread a ser criada

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

	StartServer = (BOOL(*)())GetProcAddress(hDLL, "startSpaceServer");
	StartBuff = (BOOL(*)())GetProcAddress(hDLL, "startBuffer");
	GetMSG = (Jogada(*)())GetProcAddress(hDLL, "ReadBuffer");
	wrtMSG = (BOOL(*)(Jogada))GetProcAddress(hDLL, "WriteBuffer");

	if (!StartServer()) {
		_tprintf(TEXT("Erro: n�o foi poss�vel criar Servidor -> %d\n"), GetLastError());
		return 0;
	}
	if (!StartBuff()) {
		_tprintf(TEXT("Erro: n�o foi poss�vel criar buffer -> %d\n"), GetLastError());
		return 0;
	}


	Jogada jogada, jogada1; jogada.id = 69; jogada.Dir = 69; jogada1.id = 0; jogada1.Dir = 0;
	if (!wrtMSG(jogada)) {
		_tprintf(TEXT("Erro: n�o foi poss�vel escrever do buffer -> %d\n"), GetLastError());
		return 0;
	}	


	jogada1 = GetMSG();

	_tprintf(TEXT("li da memoria  %d e a dir %d \n"), jogada1.id, jogada1.Dir );







	_tprintf(TEXT("Dificuldade (1,2,3)"));
	_tscanf_s(TEXT("%c"), &resp, 1);
	/*
	_tprintf(TEXT("Iniciar (S/N)?"));
	_tscanf_s(TEXT("%c"), &resp, 1);
	*/
	_tprintf(TEXT("Iniciar (S/N)?"));
	_tscanf_s(TEXT("%c"), &resp, 1);
	if (resp == 'S' || resp == 's') {
		

	/*	DWORD * threadId = (DWORD *)malloc(n * sizeof(DWORD));
		HANDLE * hT = (HANDLE *)malloc(n * sizeof(HANDLE));
		PARAM * p = (PARAM *)malloc(n * sizeof(PARAM));

		Mutex = CreateMutex(NULL, FALSE, TEXT("Mutex1"));//Criar Mutex
		SYSTEMTIME timei, timef;
		//GetLocalTime(&timei);
		clock_t start = clock();

		for (int i = 0; i < n; i++) {
			p[i].num = i;
			p[i].inicio = inicio + (i) * (fim - inicio) / n;
			p[i].fim = inicio + (i + 1) * (fim - inicio) / n;

			hT[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Contar, (LPVOID)&p[i], 0, &threadId[i]);
			if (hT[i] != NULL) {
				_tprintf(TEXT("Lancei uma thread com id %d\n"), threadId[i]);
				//WaitForSingleObject(hT, INFINITE);
			}
			else
				_tprintf(TEXT("Erro ao criar Thread\n"));
		}
		WaitForMultipleObjects(4, hT, TRUE, INFINITE);
		//GetLocalTime(&timef);
		/*
		FILETIME fti, ftf, ftdif;
		SystemTimeToFileTime(&timei, &fti);
		SystemTimeToFileTime(&timef, &ftf);
		*/

		/*// Execuatable code
		clock_t stop = clock();
		double elapsed = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;
		printf("Time elapsed in ms: %f", elapsed);

		_tprintf(TEXT("\n%f\n", time_spent));

		CloseHandle(Mutex);*/
	}
	_tprintf(TEXT("[Thread Principal %d]Vou terminar..."), GetCurrentThreadId());
	
	return 0;
}



/*


DWORD WINAPI thread_basica(LPVOID nave) {
	Nave *a;

	a = (Nave *)nave;
	while (a->vida > 0) {

		Sleep(a->velocidade);
		if (a->fimJanela) {

			if (can_move()) {
				if (a->fimJanela) {
					a->e.y += a->e.y + a->e.altura;
				}
				else if (a->isLeft)
					a->e.x = a->e.x - 1;
				else a->e.x = a->e.x + 1;
			}

		}

	}
	return 0;
}

DWORD WINAPI thread_esquiva(LPVOID nave) {
	Nave *b;
	b = (Nave *)nave;

	while (b->vida > 0) {
		Sleep(b->velocidade);

		//random e se sitio esta disponivel



	}

}*/