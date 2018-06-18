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
HANDLE hThreadNaveEsquiva, hThreadNaveBasica, hThreadNaveBombas, hThreadTiros;

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
DWORD WINAPI thread_tiros(LPVOID data);
DWORD WINAPI thread_bombas(LPVOID data);


//DWORD WINAPI thread_Jogo(LPVOID jogo);
#pragma endregion

#pragma region Functions
void setBomba(int x, int y);
void verifyColision(Tiro *t);
void TrataComando(Command temp);
bool CanMoveInvader(Nave *n, int x, int y);
bool verifyID(char id[], char id2[]);
void start_Jogo();
void movePlayer(Player *p, int x, int y);
void shot(Player *p);
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
	mGameAcess = CreateMutex(NULL, TRUE, _T("mutexGAME"));
	if (mGameAcess == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> MUTEX GAME\n"));
		return FALSE;
	}
	pGameView = (pJogo)MapViewOfFile(hGame, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Jogo));
	if (pGameView == NULL) {
		_tprintf(_T("(DEBUG)Server:Erro-> FILEVIEW OPEN\n"));
		return FALSE;
	}
	//mGameAcess = OpenMutex(NULL, FALSE, _T("mutexGAME"));
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
	_tprintf(TEXT("PARA TESTES DE COMS COM GATEWAY E INICIALIZAÇÃO-> Startgame? (1)"));
	_tscanf_s(_T("%d"), &resp);
	if (resp == 1) {
		start_Jogo();

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
	case LEFT:
		movePlayer(&pGameView->player[temp.id], -1, 0);
		break;
	case RIGHT:
		movePlayer(&pGameView->player[temp.id], 1, 0);
		break;
	case UP:
		movePlayer(&pGameView->player[temp.id], 0, -1);
		break;
	case DOWN:
		movePlayer(&pGameView->player[temp.id], 0, 1);
		break;
	case SHOT: //dispara tiro
		shot(&pGameView->player[temp.id]);
		break;
	default:
		_tprintf(TEXT("\n[TrataComando] Leitura: %d %d %hs\n"), temp.id, temp.cmd, temp.username);

		break;
	}

//	SetEvent(eGameUpdate);//sinaliza gateway de alterações atravez do evento
	//ResetEvent(eGameUpdate);//fecha a sinalização do evento

	ReleaseMutex(mGameAcess); //liberta mutex
	
}

void movePlayer(Player *p, int x, int y) {

	p->nave.e.x += x;
	p->nave.e.y += y;
	/*
	for (int i = 0; i < pGameView->nPlayers; i++) {
		if (p.id == id) {

		}
	}
	*/
}

void shot(Player *p) {
	for (int i = 0; i < 100; i++) {
		if (pGameView->tiros[i].e.id[0] == 'i') {
			pGameView->tiros[i].e.id[0] = 'l';
			pGameView->tiros[i].e.x = p->nave.e.x + p->nave.e.largura/2;
			pGameView->tiros[i].e.y = p->nave.e.y - 1;
			return;
		}
	}
	
}
#pragma endregion

#pragma region Jogo

#pragma region Start

void start_Jogo() {

	WaitForSingleObject(mGameAcess, INFINITE);
	

#pragma region Mapa
	pGameView->maxX = 50;
	pGameView->maxY = 200;
	pGameView->pPower = 50;
	pGameView->fBombas = 100;
	pGameView->velPoweupBomba = 1000;
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
		pGameView->nNavesNormais = 9;
		pGameView->nNavesEsquivas = 9;
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
		pGameView->navesnormais[i].i_desparo = rand() % pGameView->fBombas;
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
		pGameView->navesesquivas[i].e.altura = 3;
		pGameView->navesesquivas[i].e.largura = 3;
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
	pGameView->velTiro = 1000;

	for (int i = 0; i < 100; i++) {
		pGameView->tiros[i].e.id[0] = 'i';
	}
	for (int i = 0; i < 20; i++) {
		pGameView->bombas[i].e.id[0] = 'i';
	}
	for (int i = 0; i < 20; i++) {
		pGameView->powerups[i].e.id[0] = 'i';
	}


#pragma region Player

	for (int i = 0; i < pGameView->nPlayers; i++) {
		pGameView->player[i].nave.vida = 1;
		pGameView->player[i].nvidas = 1;
		pGameView->player[i].nave.e.altura = 3;
		pGameView->player[i].nave.e.largura = 3;
		pGameView->player[i].nave.e.id[0] = 'P';
		pGameView->player[i].nave.e.id[1] = (i + 1) / 10 + '0';
		pGameView->player[i].nave.e.id[2] = (i + 1) % 10 + '0';
	
		pGameView->player[i].nave.e.y = 47; // (int)pGameView->maxY *0.8;
		pGameView->player[i].nave.e.x = pGameView->maxX / 2;
	}

	for (int i = pGameView->nPlayers; i < 5; i++)
		pGameView->player[i].nave.vida = 0;
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

	hThreadNaveBombas = CreateThread(NULL, 0, thread_bombas, NULL, 0, 0); //inicia thread para naves basicas
	if (hThreadNaveBombas == NULL) {
		_tprintf(TEXT("[(DEBUG)Thread:Erro-> Error starting listener thread\n %d  \n"), GetLastError());
		return 0;
	}
	hThreadTiros = CreateThread(NULL, 0, thread_tiros, NULL, 0, 0); //inicia thread para naves basicas
	if (hThreadTiros == NULL) {
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
				int l = pGameView->navesnormais[i].e.largura;
				int a = pGameView->navesnormais[i].e.altura;
				//Desparar
				if (pGameView->navesnormais[i].i_desparo > 0) 
					pGameView->navesnormais[i].i_desparo--;
				else {
					pGameView->navesnormais[i].i_desparo=pGameView->fBombas;
					setBomba(x + l / 2, y + a);

				}

				//Deslocar
				if (pGameView->navesnormais[i].isLeft) {
					if (x == 0) {

						y = y + a + 1;
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



				if (CanMoveInvader(&pGameView->navesnormais[i], x, y)) {

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
#pragma region Print test


		for (int i = 0; i < 51; i++) {
			for (int j = 0; j < 201; j++) {
				Pmapa[i][j] = ' ';
			}
		}
		for (int i = 0; i < pGameView->nNavesNormais; i++) {
			for (int j = pGameView->navesnormais[i].e.x; j < (pGameView->navesnormais[i].e.x + pGameView->navesnormais[i].e.largura); j++) {
				for (int k = pGameView->navesnormais[i].e.y; k < (pGameView->navesnormais[i].e.y + pGameView->navesnormais[i].e.altura); k++) {
					Pmapa[j][k] = pGameView->navesnormais[i].e.id[2];
				}
			}
		}

		for (int i = 0; i < pGameView->nNavesEsquivas; i++) {
			for (int j = pGameView->navesesquivas[i].e.x; j < (pGameView->navesesquivas[i].e.x + pGameView->navesesquivas[i].e.largura); j++) {
				for (int k = pGameView->navesesquivas[i].e.y; k < (pGameView->navesesquivas[i].e.y + pGameView->navesesquivas[i].e.altura); k++) {
					Pmapa[j][k] = pGameView->navesesquivas[i].e.id[2];
				}
			}
		}

		for (int i = 0; i < pGameView->nPlayers; i++) {
			for (int j = pGameView->player[i].nave.e.x; j < (pGameView->player[i].nave.e.x + pGameView->player[i].nave.e.largura); j++) {
				for (int k = pGameView->player[i].nave.e.y; k < (pGameView->player[i].nave.e.y + pGameView->player[i].nave.e.altura); k++) {
					Pmapa[j][k] = 'P';
				}
			}
		}


		for (int i = 0; i < 20; i++) {
					if ((pGameView->powerups[i].e.id[0] != 'i')) {
						Pmapa[pGameView->powerups[i].e.x][pGameView->powerups[i].e.y] = pGameView->powerups[i].e.id[0];
					}
		}

		for (int i = 0; i < 20; i++) {
			if ((pGameView->bombas[i].e.id[0] != 'i')) {
				Pmapa[pGameView->bombas[i].e.x][pGameView->bombas[i].e.y] = pGameView->bombas[i].e.id[0];
			}
		}

		for (int i = 0; i < 100; i++) {
			if ((pGameView->tiros[i].e.id[0] != 'i')) {
				Pmapa[pGameView->tiros[i].e.x][pGameView->tiros[i].e.y] = 'l';
			}
		}

#pragma endregion
			
	


		for (int j = 0; j < 50; j++) {
			for (int i = 0; i < 51; i++) {
				_tprintf(TEXT("%c"), Pmapa[i][j]);
			}
			_tprintf(TEXT("\n"));
		}
		ReleaseMutex(mGameAcess);
		SetEvent(eGameUpdate);		//sinaliza gateway de alterações atravez do evento
									//Sleep(100);
		ResetEvent(eGameUpdate);	//fecha a sinalização do evento
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
					if (CanMoveInvader(&pGameView->navesesquivas[i], x, y)){
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

bool CanMoveInvader(Nave *n, int x, int y) {
	int xl = n->e.largura - 1 + x;
	int ya = n->e.altura  - 1 + y;
	char *id = n->e.id;

	if (n->vida <= 0)return false;
	
	if (x <= -1 || y <= -1)return false;
	if (xl > pGameView->maxX || ya > pGameView->maxY) return false;
	
	for (int i = 0; i < pGameView->nNavesNormais; i++) {
		if (!verifyID(id, pGameView->navesnormais[i].e.id) && pGameView->navesnormais[i].vida>0) {
			int x2 = pGameView->navesnormais[i].e.x;
			int x2l = x2 + pGameView->navesnormais[i].e.largura - 1;
			int y2 = pGameView->navesnormais[i].e.y;
			int y2a = y2 + pGameView->navesnormais[i].e.altura - 1;
			if ((y >= y2 && y <= y2a) || (ya >= y2 && ya <= y2a) || (y2 >= y && y2 <= ya) || (y2a >= y && y2a <= ya))
				if ((x >= x2 && x <= x2l) || (xl >= x2 && xl <= x2l) || (x2 >= x && x2 <= xl) || (x2l >= x && x2l <= xl))
					return false;
		}
	}
	for (int i = 0; i < pGameView->nNavesEsquivas; i++) {
		if (!verifyID(id, pGameView->navesesquivas[i].e.id) && pGameView->navesesquivas[i].vida>0) {
			int x2 = pGameView->navesesquivas[i].e.x;
			int x2l = x2 + pGameView->navesesquivas[i].e.largura - 1;
			int y2 = pGameView->navesesquivas[i].e.y;
			int y2a = y2 + pGameView->navesesquivas[i].e.altura - 1;
			if ((y >= y2 && y <= y2a) || (ya >= y2 && ya <= y2a)	|| (y2 >= y && y2 <= ya) || (y2a >= y && y2a <= ya))
				if ((x >= x2 && x <= x2l) || (xl >= x2 && xl <= x2l)|| (x2 >= x && x2 <= xl) || (x2l >= x && x2l <= xl))
					return false;
		}
	}
	return true;
}

void verifyColision(Tiro *t) {
	if (t->e.id[0] == 'i')return;
	int i, y, x, l, a;
	for (i = 0; i < pGameView->nNavesNormais; i++) {
		y = pGameView->navesnormais[i].e.y;
		x= pGameView->navesnormais[i].e.x;
		l = pGameView->navesnormais[i].e.largura;
		a = pGameView->navesnormais[i].e.altura;
		if (pGameView->navesnormais[i].vida > 0) {
			if (t->e.y >= y && t->e.y < y + a) {
				if (t->e.x >= x && t->e.x < x + l) {
					pGameView->navesnormais[i].vida -= 1;
					t->e.id[0] = 'i';
				}
			}
		}
	}

	for (i = 0; i < pGameView->nNavesEsquivas; i++) {
		y = pGameView->navesesquivas[i].e.y;
		x = pGameView->navesesquivas[i].e.x;
		l = pGameView->navesesquivas[i].e.largura;
		a = pGameView->navesesquivas[i].e.altura;
		if (pGameView->navesesquivas[i].vida > 0) {
			if (t->e.y >= y && t->e.y < y + a) {
				if (t->e.x >= x && t->e.x < x + l) {
					pGameView->navesesquivas[i].vida -= 1;
					t->e.id[0] = 'i';
				}
			}
		}

	}
}

void verifyColisionB(Bomba *b) {
	int i, y, x, l, a;

	for (i = 0; i < pGameView->nPlayers; i++) {
		y = pGameView->player[i].nave.e.y;
		x = pGameView->player[i].nave.e.x;
		l = pGameView->player[i].nave.e.largura;
		a = pGameView->player[i].nave.e.altura;

		if (b->e.y >= y && b->e.y < y + a) {
			if (b->e.x >= x && b->e.x < x + l) {
				pGameView->player[i].nave.vida -= 1;
				b->e.id[0] = 'i';
			}
		}

	}

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

void setBomba(int x, int y) {
	for (int i = 0; i < 20; i++) {
		if ((pGameView->bombas[i].e.id[0] == 'i')) {
			pGameView->bombas[i].e.id[0] = 'b';
			pGameView->bombas[i].e.x = x;
			pGameView->bombas[i].e.y = y;
			return;
		}
	}
}

#pragma endregion

DWORD WINAPI thread_tiros(LPVOID data) {
	
	while (1) {
		Sleep(pGameView->velTiro);
		WaitForSingleObject(mGameAcess,INFINITE);
		for (int i = 0; i < 100; i++) {
			if ((pGameView->tiros[i].e.id[0] != 'i')) {
				pGameView->tiros[i].e.y -= 1;
				verifyColision(&pGameView->tiros[i]);
			}
		}
		ReleaseMutex(mGameAcess);
	}
	return 0;
}


DWORD WINAPI thread_bombas(LPVOID data) {
	int random;
	srand((int)time(NULL));
	while (1) {
		Sleep(pGameView->velPoweupBomba);
		WaitForSingleObject(mGameAcess, INFINITE);
		for (int i = 0; i < 20; i++) {
			if ((pGameView->bombas[i].e.id[0] != 'i')) {
				pGameView->bombas[i].e.y += 1;
			}
		}
		random = rand() % 100;
		for (int i = 0; i < 20; i++) {
			if ((pGameView->powerups[i].e.id[0] != 'i')) {
				pGameView->powerups[i].e.y += 1;
			}
			else {
				
				if (random < pGameView->pPower) {
					int random2 = rand() % 10;
					if (random2 == 0) {
						pGameView->powerups[i].e.id[0] = 'R';

					}
					else if (random2 <= 2) {
						pGameView->powerups[i].e.id[0] = 'N';
					}
					else {
						pGameView->powerups[i].e.id[0] = 'V';
					}
					pGameView->powerups[i].e.x = rand() % pGameView->maxX-1;
					pGameView->powerups[i].e.y = 0;
					random=101;
				
				}
				
			}
		}
		ReleaseMutex(mGameAcess);
	

	}
	return 0;
}


#pragma endregion
