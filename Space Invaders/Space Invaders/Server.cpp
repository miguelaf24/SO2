#include "utils.h"

#pragma region Variáveis Globais
BOOL(*StartBuff)(void);
Command(*GetMSG)();
BOOL(*OpenGame)(void);
BOOL(*setGame)(pJogo game);
BOOL(*wrtMSG)(Command);
pBuff(*rbuff)();
HMODULE hDLL;
HANDLE hThreadListener, eGameUpdate, mGameAcess, hThreadGame;
//Jogo gameData;
HANDLE hGame;
pJogo pGameView;
#pragma endregion

#pragma region Thread
DWORD WINAPI ReadBufferThread(LPVOID params);
DWORD WINAPI thread_basica(LPVOID nave);
DWORD WINAPI thread_esquiva(LPVOID nave);

//DWORD WINAPI thread_Jogo(LPVOID jogo);
#pragma endregion

#pragma region Functions
void TrataComando(Command temp);
bool CanMoveInvader(int x, int y, TCHAR id[]);
bool verifyID(TCHAR id[], TCHAR id2[]);
void start_Jogo();
bool CanMoveInvader(int x, int y, TCHAR id[]);
#pragma endregion

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
	hGame = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("jogo"));
	if (hGame == NULL) {
		_tprintf(_T("(DEBUG)Server:Erro-> FILEMAPPING OPEN\n"));
		return FALSE;
	}

	pGameView = (pJogo)MapViewOfFile(hGame, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Jogo));
	if (pGameView == NULL) {
		_tprintf(_T("(DEBUG)Server:Erro-> FILEVIEW OPEN\n"));
		return FALSE;
	}
	mGameAcess = OpenMutex(NULL, FALSE, _T("mutexGAME"));
	eGameUpdate= CreateEvent(NULL, TRUE, FALSE, TEXT("GameUpdateEvent"));
	StartBuff = (BOOL(*)())GetProcAddress(hDLL, "startBuffer");
	GetMSG = (Command(*)())GetProcAddress(hDLL, "ReadBuffer");
	wrtMSG = (BOOL(*)(Command))GetProcAddress(hDLL, "WriteBuffer");
	setGame = (BOOL(*)(pJogo))GetProcAddress(hDLL, "setGame"); //declaração função do DLL


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
	pGameView->dificuldade = resp;

	start_Jogo();
	
	/*hThreadGame = CreateThread(NULL, 0, thread_Jogo, NULL, 0, 0);
	if (hThreadGame == NULL) {
		_tprintf(TEXT("[(DEBUG)Thread:Erro-> Error starting listener thread\n %d  \n"), GetLastError());
		return 0;
	}*/


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

#pragma region Buffer
DWORD WINAPI ReadBufferThread(LPVOID params) {
	
	Command temp;

	while (1) {

		temp = GetMSG(); //inicia a função do dll que aguarda pelo semaforo;
		_tprintf(TEXT("\n[THREAD] Leitura: %d %d\n"), temp.id, temp.cmd);


		//CHAMA FUNÇÃO DE MANIPULAÇÂO DE COMANDOS
		//TrataComando(temp);
	}
	
}

void TrataComando(Command temp) {
	
	WaitForSingleObject(mGameAcess, INFINITE); //aguarda disponibilidade do mutex de acesso aos dados de jogo
		
	//trata de tudo o que tem a ver com o comando;
	//TODO bla bla bla

	SetEvent(eGameUpdate);//sinaliza gateway de alterações atravez do evento
	ResetEvent(eGameUpdate);//fecha a sinalização do evento

	ReleaseMutex(mGameAcess); //liberta mutex
	
}
#pragma endregion

#pragma region Jogo

#pragma region Start

void start_Jogo() {
	Jogo j;
	

	HANDLE hThreadNaveEsquiva, hThreadNaveBasica;
	WaitForSingleObject(mGameAcess, INFINITE);
	

	#pragma region Mapa
	pGameView->maxX = 50;
	pGameView->maxY = 200;
	#pragma endregion
	
	#pragma region Dificuldade
	switch (pGameView->dificuldade)
	{
	case 1:
		pGameView->nNavesNormais = 10;
		pGameView->nNavesEsquivas = 5;
		break;
	case 2:
		pGameView->nNavesNormais = 20;
		pGameView->nNavesEsquivas = 10;
		break;
	case 3:
		pGameView->nNavesNormais = 30;
		pGameView->nNavesEsquivas = 15;
		break;
	default:
		break; 
	}
	#pragma endregion
	
	#pragma region Naves Normais
	for (int i = 0; i < pGameView->nNavesNormais; i++) {
		pGameView->navesnormais[i].velocidade = 1000 - 100*(pGameView->dificuldade - 1);
		pGameView->navesnormais[i].vida = 1;
		pGameView->navesnormais[i].e.altura = 3;
		pGameView->navesnormais[i].e.largura = 3;
		pGameView->navesnormais[i].e.id[0] = 'N';
		pGameView->navesnormais[i].e.id[1] = (i+1)/10 + '0';
		pGameView->navesnormais[i].e.id[2] = (i+1)%10 + '0';
		if (i > pGameView->nNavesEsquivas / 2)
			pGameView->navesnormais[i].e.y = 6;
		else 
			pGameView->navesnormais[i].e.y = 9;
		pGameView->navesnormais[i].e.x = 4 * i;	
	}

	for (int i = pGameView->nNavesNormais; i < 30; i++) 
		pGameView->navesnormais[i].vida = 0;
	#pragma endregion
	
	#pragma region Naves Esquivas
	for (int i = 0; i < pGameView->nNavesEsquivas; i++) {
		pGameView->navesesquivas[i].e.altura = 3;
		pGameView->navesesquivas[i].e.largura = 3;
		if (i > pGameView->nNavesEsquivas/2)
			pGameView->navesesquivas[i].e.y = 0;
		else
			pGameView->navesesquivas[i].e.y = 3;
		pGameView->navesesquivas[i].e.x = 4 * i;
		pGameView->navesesquivas[i].velocidade = (1000 - 100 * (pGameView->dificuldade - 1))*1.1;
		pGameView->navesesquivas[i].vida = 3;
	}
	
	for (int i = pGameView->nNavesEsquivas; i < 30; i++)
		pGameView->navesesquivas[i].vida = 0;
	#pragma endregion

	#pragma region Memória Partilhada

	//actualiza memoria partilhada
	SetEvent(eGameUpdate);//sinaliza gateway de alterações atravez do evento

    ResetEvent(eGameUpdate);//fecha a sinalização do evento
		//CopyMemory(pGameView, &j, sizeof(Jogo));
	ReleaseMutex(mGameAcess);

	#pragma endregion
	
	#pragma region Create Threads

	hThreadNaveEsquiva = CreateThread(NULL, 0, thread_esquiva, NULL, 0, 0); //inicia thread para naves esquivas
	if (hThreadNaveEsquiva == NULL) {
		_tprintf(TEXT("[(DEBUG)Thread:Erro-> Error starting listener thread\n %d  \n"), GetLastError());
		return;
	}

	hThreadNaveBasica = CreateThread(NULL, 0, thread_basica, NULL, 0, 0); //inicia thread para naves basicas
	if (hThreadNaveBasica == NULL) {
		_tprintf(TEXT("[(DEBUG)Thread:Erro-> Error starting listener thread\n %d  \n"), GetLastError());
		return;
	}
	
	#pragma endregion
	
	//TODO-> TRATAMENTO DE JOGO, POWERUPS E TIROS ATACANTES

	return;
}

#pragma endregion

#pragma region Naves
DWORD WINAPI thread_basica(LPVOID nave) {

	while (pGameView->nNavesNormais > 0) {
		
		Sleep(pGameView->navesnormais->velocidade);
		WaitForSingleObject(mGameAcess, INFINITE);
		for (int i = 0; i < pGameView->nNavesNormais; i++) {
			if (pGameView->navesnormais[i].vida > 0) {
				int x = pGameView->navesnormais[i].e.x;
				int y = pGameView->navesnormais[i].e.y;
				if (pGameView->navesnormais[i].isLeft) {
					if (x == 0) {
						y = y + pGameView->navesnormais[i].e.altura;
						pGameView->navesnormais[i].isLeft = false;
					}
					else
						x = x - 1;
				}
				else {
					if (x == pGameView->maxX) {
						y = y + pGameView->navesnormais[i].e.altura;
						pGameView->navesnormais[i].isLeft = true;
					}
					else
						x = x + 1;
				}

				
				
				if (CanMoveInvader(x, y, pGameView->navesnormais[i].e.id)) {
					pGameView->navesnormais[i].e.y = y;
					pGameView->navesnormais[i].e.x = x;
				}
				
			}

		}
		ReleaseMutex(mGameAcess);
		
		SetEvent(eGameUpdate);//sinaliza gateway de alterações atravez do evento
		//Sleep(100);
		ResetEvent(eGameUpdate);//fecha a sinalização do evento
	}

	return 0;
}

DWORD WINAPI thread_esquiva(LPVOID nave) {

	

//	Nave *n = gameData.navesesquivas;
	
	while (pGameView->nNavesEsquivas > 0) {
		//Sleep(n->velocidade);
		WaitForSingleObject(mGameAcess, INFINITE);
	
		//TODO-> Meter movimento random e verificações
		ReleaseMutex(mGameAcess);
	}
	return 0;
}

bool CanMoveInvader(int x, int y, TCHAR id[]) {
	for (int i = 0; i < pGameView->nNavesNormais; i++) {
		if (!verifyID(id, pGameView->navesnormais[i].e.id)) {
			if (x >= pGameView->navesnormais[i].e.x && x <= pGameView->navesnormais[i].e.x + pGameView->navesnormais[i].e.largura)
				if (y >= pGameView->navesnormais[i].e.y && x <= pGameView->navesnormais[i].e.y + pGameView->navesnormais[i].e.altura)
					return false;
		}
	}
	return false;
}

bool verifyID(TCHAR id[], TCHAR id2[]) {
	for (int i = 0; i < 3; i++) {
		if (id[i] != id2[i])
			return false;
	}
	return true;
}

#pragma endregion

#pragma endregion
