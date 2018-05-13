#include "utils.h"
BOOL(*StartServer)(void);
BOOL(*StartBuff)(void);
Command(*GetMSG)();
BOOL(*setGame)(pJogo game);
BOOL(*wrtMSG)(Command);
pBuff(*rbuff)();
HMODULE hDLL;
HANDLE hThreadListener, eGameUpdate, mGameAcess, hThreadGame;
Jogo gameData;


//Threads
DWORD WINAPI ReadBufferThread(LPVOID params);
DWORD WINAPI thread_Jogo(LPVOID jogo);

//FUNCTIONS
void TrataComando(Command temp);
bool CanMoveInvader(int x, int y);


int _tmain(int argc, LPTSTR argv[]) {
	int resp;

	//DWORD threadId[]; //Id da thread a ser criada
	//HANDLE hT[4] = (HANDLE *)malloc(4 * sizeof(HANDLE)); //HANDLE/ponteiro para a thread a ser criada

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif
	srand((int)time(NULL));

	//DLL LOAD
	hDLL = LoadLibrary(_T("SpaceDLL"));
	if (hDLL == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> Loading DLL\n"));
		return 0;
	}
	mGameAcess = CreateMutex(NULL, FALSE, _T("mGameAcess"));
	eGameUpdate= CreateEvent(NULL, TRUE, FALSE, TEXT("GameUpdateEvent"));
	StartServer = (BOOL(*)())GetProcAddress(hDLL, "startSpaceServer");
	StartBuff = (BOOL(*)())GetProcAddress(hDLL, "startBuffer");
	GetMSG = (Command(*)())GetProcAddress(hDLL, "ReadBuffer");
	wrtMSG = (BOOL(*)(Command))GetProcAddress(hDLL, "WriteBuffer");
	setGame = (BOOL(*)(pJogo))GetProcAddress(hDLL, "setGame"); //declaração função do DLL
	gameData.dificuldade = 0;//TESTE APENAS

	if (!StartServer()) { //incia server, e memoria partilhada
		_tprintf(TEXT("(DEBUG)Server:Erro-> Starting Server\n %d\n"), GetLastError());
		return 0;
	}
	if (!StartBuff()) { //inicia buffer de mensagens
		_tprintf(TEXT("(DEBUG)Server:Erro-> Starting Buffer\n %d\n"), GetLastError());
		return 0;
	}

	hThreadListener = CreateThread(NULL,0, ReadBufferThread,NULL,0,0);
	if (hThreadListener == NULL) {
		_tprintf(TEXT("[(DEBUG)Thread:Erro-> Error starting listener thread\n %d  \n"), GetLastError());
		return 0;
	}

	_tprintf(TEXT("PARA TESTES DE COMS COM GATEWAY E INICIALIZAÇÃO-> Insira dificuldade (1,2,3)"));
	_tscanf_s(_T("%d"), &resp);
	
	gameData.dificuldade = resp;
	setGame(&gameData);
	
	hThreadGame = CreateThread(NULL, 0, thread_Jogo, NULL, 0, 0);
	if (hThreadGame == NULL) {
		_tprintf(TEXT("[(DEBUG)Thread:Erro-> Error starting listener thread\n %d  \n"), GetLastError());
		return 0;
	}


	/*_tprintf(TEXT("Iniciar (S/N)?"));
	_tscanf_s(TEXT("%c"), &resp, 1);*/


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


	WaitForSingleObject(hThreadListener, INFINITE);
	_tprintf(TEXT("[Thread Principal %d]Vou terminar..."), GetCurrentThreadId());
	

	return 0;
}

DWORD WINAPI ReadBufferThread(LPVOID params) {
	
	Command temp;

	while (1) {

		temp = GetMSG(); //inicia a função do dll que aguarda pelo semaforo;
		_tprintf(TEXT("\n[THREAD] Leitura: %d %d\n"), temp.id, temp.cmd);


		//CHAMA FUNÇÃO DE MANIPULAÇÂO DE COMANDOS
		TrataComando(temp);
	}

}

void TrataComando(Command temp) {
	
	WaitForSingleObject(mGameAcess, INFINITE); //aguarda disponibilidade do mutex de acesso aos dados de jogo
		
	//trata de tudo o que tem a ver com o comando;
	//TODO bla bla bla

	setGame(&gameData);// grava as alterações na memoria partilhada
	SetEvent(eGameUpdate);//sinaliza gateway de alterações atravez do evento
	ResetEvent(eGameUpdate);//fecha a sinalização do evento

	ReleaseMutex(mGameAcess); //liberta mutex
	
}




DWORD WINAPI thread_basica(LPVOID nave) {

	

	Nave *n = gameData.navesnormais;
	

	while (gameData.nNavesNormais > 0) {
		Sleep(n[0].velocidade);
		for (int i = 0; i < gameData.nNavesNormais; i++) {
			if (n[i].vida > 0) {
				int x = n[i].e.x;
				int y = n[i].e.y;
				if (n[i].fimJanela) {
					y = y + n[i].e.altura;
				}
				else if (n[i].isLeft)
					x = x - 1;
				else x = x + 1;

				//_tprintf(TEXT("\n[THREAD] Nave[%d , %d]\n"),x, y);
				
				if (CanMoveInvader(x, y)) {
					n[i].e.y = y;
					n[i].e.x = x;
				}
				
			}

		}
	}
	return 0;
}

DWORD WINAPI thread_esquiva(LPVOID nave) {

	

	Nave *n = gameData.navesesquivas;
	
	while (gameData.nNavesEsquivas > 0) {
//		Sleep(n->velocidade);
		//TODO-> Meter movimento random e verificações

	}
	return 0;
}

bool CanMoveInvader(int x, int y) {
	for (int i = 0; i < gameData.nNavesNormais; i++) {
		//TODO-> Verifica se determinada posição esta ocupada
	}
	return TRUE;
}

DWORD WINAPI thread_Jogo(LPVOID jogo) {
	pJogo j;
	j = (pJogo)&gameData;

	HANDLE hThreadNaveEsquiva, hThreadNaveBasica;

	switch (j->dificuldade)
	{
	case 1:
		j->nNavesNormais = 10;
		j->nNavesEsquivas = 5;
		gameData.navesnormais = (Nave *)malloc(sizeof(Nave)*j->nNavesNormais);
		gameData.navesesquivas = (Nave *)malloc(sizeof(Nave)*j->nNavesEsquivas);
		break;
	case 2:
		j->nNavesNormais = 20;
		j->nNavesEsquivas = 10;

		gameData.navesnormais = (Nave *)malloc(sizeof(Nave)*j->nNavesNormais);
		gameData.navesesquivas = (Nave *)malloc(sizeof(Nave)*j->nNavesEsquivas);
		break;
	case 3:
		j->nNavesNormais = 30;
		j->nNavesEsquivas = 15;

		gameData.navesnormais = (Nave *)malloc(sizeof(Nave)*j->nNavesNormais);
		gameData.navesesquivas = (Nave *)malloc(sizeof(Nave)*j->nNavesEsquivas);
		break;
	default:
		break; 
	}

	Nave *nNormal = gameData.navesnormais;
	Nave *nEsquiva = gameData.navesesquivas;
		
	for (int i = 0; i < j->nNavesNormais; i++) {
		nNormal[i].velocidade = 1000 - 100*(j->dificuldade - 1);
		nNormal[i].vida = 1;
		nNormal[i].e.altura = 3;
		nNormal[i].e.largura = 3;
		if (i > j->nNavesEsquivas / 2)
			nNormal[i].e.y = 6;
		else 
			nNormal[i].e.y = 9;
		nNormal[i].e.x = 4 * i;
	}
	for (int i = 0; i < j->nNavesEsquivas; i++) {
		nEsquiva[i].e.altura = 3;
		nEsquiva[i].e.largura = 3;
		if (i > j->nNavesEsquivas/2)
			nEsquiva[i].e.y = 0;
		else
			nEsquiva[i].e.y = 3;
		nEsquiva[i].e.x = 4 * i;
		nEsquiva[i].velocidade = (1000 - 100 * (j->dificuldade - 1))*1.1;
		nEsquiva[i].vida = 3;

	}
	
	hThreadNaveEsquiva = CreateThread(NULL, 0, thread_esquiva, NULL, 0, 0); //inicia thread para naves esquivas
	if (hThreadNaveEsquiva == NULL) {
		_tprintf(TEXT("[(DEBUG)Thread:Erro-> Error starting listener thread\n %d  \n"), GetLastError());
		return 0;
	}

	hThreadNaveBasica = CreateThread(NULL, 0, thread_basica, NULL, 0, 0); //inicia thread para naves basicas
	if (hThreadNaveBasica == NULL) {
		_tprintf(TEXT("[(DEBUG)Thread:Erro-> Error starting listener thread\n %d  \n"), GetLastError());
		return 0;
	}
	
	//TODO-> TRATAMENTO DE JOGO, POWERUPS E TIROS ATACANTES

	return 0;

}