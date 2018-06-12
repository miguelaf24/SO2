#include "utils.h"

#pragma region Variáveis Globais
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
char Pmapa[51][201];

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
	_tprintf(TEXT("PARA TESTES DE COMS COM GATEWAY E INICIALIZAÇÃO-> Startgame? (1)"));
	_tscanf_s(_T("%d"), &resp);
	if (resp == 1) {

		SetEvent(eGameStart);//sinaliza gateway de alterações atravez do evento
		ResetEvent(eGameStart);//fecha a sinalização do evento
		SetEvent(eGameUpdate);//sinaliza gateway de alterações atravez do evento
		ResetEvent(eGameUpdate);//fecha a sinalização do evento
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

		 GetMSG(temp); //inicia a função do dll que aguarda pelo semaforo;
		_tprintf(TEXT("\n[THREAD] Leitura: %d %d %hs\n"), temp->id, temp->cmd, temp->username);

		
		if (temp->cmd == 0) {
			_tprintf(TEXT("\n[THREAD] Bem vindo:%hs\n"), temp->username);
		}

		//CHAMA FUNÇÃO DE MANIPULAÇÂO DE COMANDOS
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
//	SetEvent(eGameUpdate);//sinaliza gateway de alterações atravez do evento
	//ResetEvent(eGameUpdate);//fecha a sinalização do evento

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
		pGameView->navesnormais[i].velocidade = 1000 - 100 * (pGameView->dificuldade - 1);
		pGameView->navesnormais[i].vida = 1;
		pGameView->navesnormais[i].e.altura = 3;
		pGameView->navesnormais[i].e.largura = 3;
		pGameView->navesnormais[i].e.id[0] = 'N';
		pGameView->navesnormais[i].e.id[1] = (i + 1) / 10 + '0';
		pGameView->navesnormais[i].e.id[2] = (i + 1) % 10 + '0';
		auxX += 4;
		if (auxX + pGameView->navesnormais[i].e.largura >= pGameView->maxX) {
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
	auxX = 0;
	auxY = 18;
	for (int i = 0; i < pGameView->nNavesEsquivas; i++) {
		pGameView->navesesquivas[i].e.altura = 1;
		pGameView->navesesquivas[i].e.largura = 1;
		auxX += 8;
		if (auxX + pGameView->navesesquivas[i].e.largura >= pGameView->maxX) {
			auxY += 4;
			auxX = 4;
			auxLeft = !auxLeft;
		}

		pGameView->navesesquivas[i].e.id[0] = 'E';
		pGameView->navesesquivas[i].e.id[1] = (i + 1) / 10 + '0';
		pGameView->navesesquivas[i].e.id[2] = (i + 1) % 10 + '0';
		pGameView->navesesquivas[i].e.y = auxY;
		pGameView->navesesquivas[i].e.x = auxX;
		pGameView->navesesquivas[i].velocidade = (1000 - 100 * (pGameView->dificuldade - 1))*1.1;
		pGameView->navesesquivas[i].vida = 3;
	}

	for (int i = pGameView->nNavesEsquivas; i < 30; i++)
		pGameView->navesesquivas[i].vida = 0;
#pragma endregion

	

	ReleaseMutex(mGameAcess);



	
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

	while (pGameView->nNavesNormais > 0) {

		Sleep((DWORD)pGameView->navesnormais->velocidade);
		WaitForSingleObject(mGameAcess, INFINITE);

		for (int i = pGameView->nNavesNormais - 1; i >= 0; i--) {
			if (pGameView->navesnormais[i].vida > 0) {
				int x = pGameView->navesnormais[i].e.x;
				int y = pGameView->navesnormais[i].e.y;
				if (pGameView->navesnormais[i].isLeft) {
					if (x == 0) {

						y = y + pGameView->navesnormais[i].e.altura + 1;
					}
					else
						x = x - 1;
				}
				else {
					if (x + pGameView->navesnormais[i].e.largura == pGameView->maxX) {
						y = y + pGameView->navesnormais[i].e.altura + 1;
					}
					else
						x = x + 1;
				}



				if (CanMoveInvader(x, y, x + pGameView->navesnormais[i].e.largura - 1, y + pGameView->navesnormais[i].e.altura - 1, pGameView->navesnormais[i].e.id)) {

					if (pGameView->navesnormais[i].e.x == 0) {
						pGameView->navesnormais[i].isLeft = false;
					}

					if (pGameView->navesnormais[i].e.x + pGameView->navesnormais[i].e.largura == pGameView->maxX) {
						pGameView->navesnormais[i].isLeft = true;
					}
					pGameView->navesnormais[i].e.y = y;
					pGameView->navesnormais[i].e.x = x;
				}

			}

		}
		ReleaseMutex(mGameAcess);

		for (int i = 0; i < 51; i++) {
			for (int j = 0; j < 201; j++) {
				Pmapa[i][j] = ' ';
			}
		}
		for (int i = 0; i < pGameView->nNavesNormais; i++) {
			for (int j = pGameView->navesnormais[i].e.x; j < (pGameView->navesnormais[i].e.x + pGameView->navesnormais[i].e.largura); j++) {
				for (int k = pGameView->navesnormais[i].e.y; k < (pGameView->navesnormais[i].e.y + pGameView->navesnormais[i].e.altura); k++) {
					Pmapa[j][k] = 'N';
				}
			}
		}

		for (int i = 0; i < pGameView->nNavesEsquivas; i++) {
			for (int j = pGameView->navesesquivas[i].e.x; j < (pGameView->navesesquivas[i].e.x + pGameView->navesesquivas[i].e.largura); j++) {
				for (int k = pGameView->navesesquivas[i].e.y; k < (pGameView->navesesquivas[i].e.y + pGameView->navesesquivas[i].e.altura); k++) {
					Pmapa[j][k] = 'E';
				}
			}
		}

		for (int j = 0; j < 50; j++) {
			for (int i = 0; i < 51; i++) {
				_tprintf(TEXT("%c"), Pmapa[i][j]);
			}
			_tprintf(TEXT("\n"));
		}
		SetEvent(eGameUpdate);//sinaliza gateway de alterações atravez do evento
							  //Sleep(100);
		ResetEvent(eGameUpdate);//fecha a sinalização do evento
	}

	return 0;
}

DWORD WINAPI thread_esquiva(LPVOID nave) {



	//	Nave *n = gameData.navesesquivas;

	while (pGameView->nNavesEsquivas > 0) {
		Sleep((DWORD)pGameView->navesesquivas->velocidade);
		WaitForSingleObject(mGameAcess, INFINITE);

		for (int i = pGameView->nNavesEsquivas - 1; i > -1; i--) {
			if (pGameView->navesesquivas[i].vida > 0) {


				int random = rand() % 4;
				for (int j = 0; j < 4; j++) {
					int x = pGameView->navesesquivas[i].e.x;
					int y = pGameView->navesesquivas[i].e.y;
					random = random + j;
					if (random >= 4) random -= 4;
					switch (random)
					{
					case 0:
						x++;
						break;
					case 1:
						x--;
						break;
					case 2:
						y++;
						break;
					case 3:
						y--;
						break;
					default:
						break;
					}
					if (CanMoveInvader(x, y, x + pGameView->navesesquivas[i].e.largura - 1, y + pGameView->navesesquivas[i].e.altura - 1, pGameView->navesesquivas[i].e.id)) {
						pGameView->navesesquivas[i].e.y = y;
						pGameView->navesesquivas[i].e.x = x;
						break;
					}
				}

			}
		}
	
		ReleaseMutex(mGameAcess);
	}
	return 0;
}

bool CanMoveInvader(int x, int y, int xl, int ya, char id[]) {
	if (x <= -1 || y <= -1)return false;
	if (xl > pGameView->maxX || ya > pGameView->maxY) return false;
	for (int i = 0; i < pGameView->nNavesNormais; i++) {
		if (!verifyID(id, pGameView->navesnormais[i].e.id)) {
			int x2 = pGameView->navesnormais[i].e.x;
			int x2l = x2 + pGameView->navesnormais[i].e.largura - 1;
			int y2 = pGameView->navesnormais[i].e.y;
			int y2a = y2 + pGameView->navesnormais[i].e.altura - 1;
			if ((y >= y2 && y <= y2a) || (ya >= y2 && ya <= y2a))
				if ((x >= x2 && x <= x2l) || (xl >= x2 && xl <= x2l))
					return false;
		}
	}
	for (int i = 0; i < pGameView->nNavesEsquivas; i++) {
		if (!verifyID(id, pGameView->navesesquivas[i].e.id)) {
			int x2 = pGameView->navesesquivas[i].e.x;
			int x2l = x2 + pGameView->navesesquivas[i].e.largura - 1;
			int y2 = pGameView->navesesquivas[i].e.y;
			int y2a = y2 + pGameView->navesesquivas[i].e.altura - 1;
			if ((y >= y2 && y <= y2a) || (ya >= y2 && ya <= y2a))
				if ((x >= x2 && x <= x2l) || (xl >= x2 && xl <= x2l))
					return false;
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
