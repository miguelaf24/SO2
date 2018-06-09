#include "utils.h"

#pragma region Vari�veis Globais
BOOL(*StartBuff)(void);
BOOL(*GetMSG)(pCommand);
BOOL(*OpenGame)(void);
BOOL(*setGame)(pJogo game);
BOOL(*wrtMSG)(Command);
pBuff(*rbuff)();
HMODULE hDLL;
HANDLE hThreadListener, eGameUpdate, mGameAcess, hThreadGame, eGameStart;
HANDLE hThreadNaveEsquiva, hThreadNaveBasica;

//Jogo gameData;
HANDLE hGame;
pJogo pGameView;

#pragma endregion

#pragma region Thread
DWORD WINAPI ReadBufferThread(LPVOID params);
DWORD WINAPI thread_basica(LPVOID nave);
DWORD WINAPI thread_esquiva(LPVOID nave);
DWORD WINAPI thread_Jogo(LPVOID nave);


//DWORD WINAPI thread_Jogo(LPVOID jogo);
#pragma endregion

#pragma region Functions
void TrataComando(Command temp);
bool CanMoveInvader(int x, int y, int xl, int ya, char id[]);
bool verifyID(char id[], char id2[]);
void start_Jogo();
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
	eGameStart = CreateEvent(NULL, TRUE, FALSE, TEXT("GameStartEvent"));

	StartBuff = (BOOL(*)())GetProcAddress(hDLL, "startBuffer");
	GetMSG = (BOOL(*)(pCommand))GetProcAddress(hDLL, "ReadBuffer");
	wrtMSG = (BOOL(*)(Command))GetProcAddress(hDLL, "WriteBuffer");
	setGame = (BOOL(*)(pJogo))GetProcAddress(hDLL, "setGame"); //declara��o fun��o do DLL


	if (!StartBuff()) { //inicia buffer de mensagens
		_tprintf(TEXT("(DEBUG)Server:Erro-> Starting Buffer\n %d\n"), GetLastError());
		return 0;
	}

	hThreadListener = CreateThread(NULL,0, ReadBufferThread,NULL,0,0);
	if (hThreadListener == NULL) {
		_tprintf(TEXT("[(DEBUG)Thread:Erro-> Error starting listener thread\n %d  \n"), GetLastError());
		return 0;
	}

	_tprintf(TEXT("PARA TESTES DE COMS COM GATEWAY E INICIALIZA��O-> Insira dificuldade (1,2,3)"));
	_tscanf_s(_T("%d"), &resp);
	pGameView->dificuldade = resp;
	start_Jogo();
	_tprintf(TEXT("PARA TESTES DE COMS COM GATEWAY E INICIALIZA��O-> Startgame? (1)"));
	_tscanf_s(_T("%d"), &resp);
	if (resp == 1) {

		SetEvent(eGameStart);//sinaliza gateway de altera��es atravez do evento
		ResetEvent(eGameStart);//fecha a sinaliza��o do evento
		SetEvent(eGameUpdate);//sinaliza gateway de altera��es atravez do evento
		ResetEvent(eGameUpdate);//fecha a sinaliza��o do evento
		hThreadGame = CreateThread(NULL, 0, thread_Jogo, NULL, 0, 0);
		if (hThreadGame == NULL) {
			_tprintf(TEXT("[(DEBUG)Thread:Erro-> Error starting listener thread\n %d  \n"), GetLastError());
			return 0;
		}

	}


	
	WaitForSingleObject(hThreadListener, INFINITE);
	CloseHandle(hThreadListener);
	CloseHandle(hThreadGame);
	CloseHandle(hThreadNaveBasica);
	CloseHandle(hThreadNaveEsquiva);

	
	_tprintf(TEXT("[Thread Principal %d]Vou terminar..."), GetCurrentThreadId());
	

	return 0;
}





#pragma region Buffer
DWORD WINAPI ReadBufferThread(LPVOID params) {
	
	pCommand temp= (pCommand)malloc(sizeof(Command));
	
	
	while (1) {

		 GetMSG(temp); //inicia a fun��o do dll que aguarda pelo semaforo;
		_tprintf(TEXT("\n[THREAD] Leitura: %d %d %hs\n"), temp->id, temp->cmd, temp->username);

		
		if (temp->cmd == 0) {
			_tprintf(TEXT("\n[THREAD] Bem vindo:%hs\n"), temp->username);
		}

		//CHAMA FUN��O DE MANIPULA��O DE COMANDOS
		TrataComando(*temp);
	}
	
}

void TrataComando(Command temp) {
	
	WaitForSingleObject(mGameAcess, INFINITE); //aguarda disponibilidade do mutex de acesso aos dados de jogo
		
	//trata de tudo o que tem a ver com o comando;
	//TODO bla bla bla
	switch (temp.cmd) {
	case 0:

		pGameView->nPlayers++;
		pGameView->player[temp.id].id = temp.id;
		CopyMemory(pGameView->player[temp.id].username,temp.username, sizeof(temp.username));
		break;
	case 1:
		
		break;

	default:
		_tprintf(TEXT("\n[TrataComando] Leitura: %d %d %hs\n"), temp.id, temp.cmd, temp.username);

		break;
	}
//	SetEvent(eGameUpdate);//sinaliza gateway de altera��es atravez do evento
	//ResetEvent(eGameUpdate);//fecha a sinaliza��o do evento

	ReleaseMutex(mGameAcess); //liberta mutex
	
}
#pragma endregion

#pragma region Jogo

#pragma region Start

void start_Jogo() {

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

	int auxX = 0;
	int auxY = 6;
	bool auxLeft = true;
	for (int i = 0; i < pGameView->nNavesNormais; i++) {
		pGameView->navesnormais[i].velocidade = 1000 - 100*(pGameView->dificuldade - 1);
		pGameView->navesnormais[i].vida = 1;
		pGameView->navesnormais[i].e.altura = 3;
		pGameView->navesnormais[i].e.largura = 3;
		pGameView->navesnormais[i].e.id[0] = 'N';
		pGameView->navesnormais[i].e.id[1] = (i+1)/10 + '0';
		pGameView->navesnormais[i].e.id[2] = (i+1)%10 + '0';
		auxX += 4;
		if (auxX + pGameView->navesnormais[i].e.largura >= pGameView->maxX ) {
			auxY += 4;
			auxX = 4;
			auxLeft = !auxLeft;
		}
		pGameView->navesnormais[i].e.y = auxY;
		pGameView->navesnormais[i].e.x = auxX;	
		pGameView->navesnormais[i].isLeft = auxLeft;
	}

	for (int i = pGameView->nNavesNormais; i < 30; i++) 
		pGameView->navesnormais[i].vida = 0;
	#pragma endregion
	
	#pragma region Naves Esquivas
	for (int i = 0; i < pGameView->nNavesEsquivas; i++) {
		pGameView->navesesquivas[i].e.altura = 1;
		pGameView->navesesquivas[i].e.largura = 1;
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

	#pragma region Mem�ria Partilhada

	//actualiza memoria partilhada
/*	SetEvent(eGameUpdate);//sinaliza gateway de altera��es atravez do evento

	ResetEvent(eGameUpdate);//fecha a sinaliza��o do evento*/
		//CopyMemory(pGameView, &j, sizeof(Jogo));
	ReleaseMutex(mGameAcess);

	#pragma endregion
	

	
	//TODO-> TRATAMENTO DE JOGO, POWERUPS E TIROS ATACANTES

	return;
}



#pragma region Create Threads
DWORD WINAPI thread_Jogo(LPVOID nave) {


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

	return 0;
}
#pragma endregion


#pragma endregion

#pragma region Naves
DWORD WINAPI thread_basica(LPVOID nave) {
	_tprintf(TEXT("[(DEBUG)threads  basica \n"));
	while (pGameView->nNavesNormais > 0) {
		_tprintf(TEXT("[(DEBUG)NAVE BASICA ACTUALIZA JOGO  \n"));

		Sleep(2000);//(DWORD)pGameView->navesnormais->velocidade );
		WaitForSingleObject(mGameAcess, INFINITE);
		for (int i = pGameView->nNavesNormais - 1; i >= 0 ; i--) {
			if (pGameView->navesnormais[i].vida > 0) {
				int x = pGameView->navesnormais[i].e.x;
				int y = pGameView->navesnormais[i].e.y;
				if (pGameView->navesnormais[i].isLeft) {
					if (x == 0) {

						y = y + pGameView->navesnormais[i].e.altura + 1;
						pGameView->navesnormais[i].isLeft = false;
					}
					else
						x = x - 1;
				}
				else {
					if (x + pGameView->navesnormais[i].e.largura == pGameView->maxX ) {
						y = y + pGameView->navesnormais[i].e.altura + 1;
						pGameView->navesnormais[i].isLeft = true;
					}
					else
						x = x + 1;
				}

				
				
				if (CanMoveInvader(x, y, x + pGameView->navesnormais[i].e.largura, x + pGameView->navesnormais[i].e.altura, pGameView->navesnormais[i].e.id)) {
					pGameView->navesnormais[i].e.y = y;
					pGameView->navesnormais[i].e.x = x;

					
				}
				else {
					if (x == 0) {
						pGameView->navesnormais[i].isLeft = true;

					}
					if (x + pGameView->navesnormais[i].e.largura == pGameView->maxX) {

						pGameView->navesnormais[i].isLeft = false;
					}
				}
				
			}

		}
		ReleaseMutex(mGameAcess);
		

		SetEvent(eGameUpdate);//sinaliza gateway de altera��es atravez do evento
		//Sleep(100);
		ResetEvent(eGameUpdate);//fecha a sinaliza��o do evento
	}

	return 0;
}

DWORD WINAPI thread_esquiva(LPVOID nave) {

	

//	Nave *n = gameData.navesesquivas;
	
	while (pGameView->nNavesEsquivas > 0) {
		//Sleep(n->velocidade);
		WaitForSingleObject(mGameAcess, INFINITE);
	
		//TODO-> Meter movimento random e verifica��es
		ReleaseMutex(mGameAcess);
	}
	return 0;
}

bool CanMoveInvader(int x, int y, int xl, int ya, char id[]) {
	for (int i = 0; i < pGameView->nNavesNormais; i++) {
		if (!verifyID(id, pGameView->navesnormais[i].e.id)) {
			int x2 = pGameView->navesnormais[i].e.x;
			int x2l = x2 + pGameView->navesnormais[i].e.largura;
			int y2 = pGameView->navesnormais[i].e.y;
			int y2a = y2 + pGameView->navesnormais[i].e.altura;
			if (x >= x2 && x <= x2l && xl >= x2 && xl <= x2l)
				if (y >= y2 && y <= y2a && ya >= y2 && ya <= y2a) {
					_tprintf(_T("Dentro - True\n"));
					return false;
				}
		}
	}
	return true;
}

bool verifyID(char id[], char id2[]) {
	for (int i = 0; i < 3; i++) {
		if (id[i] != id2[i]) {
			//_tprintf(_T("ID - False\n"));
			return false;
		}
	}
	//_tprintf(_T("ID - True\n"));
	return true;
}

#pragma endregion

#pragma endregion
