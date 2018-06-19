#include "utils.h"
#include "stdafx.h"
#include "resource.h"

#define MAX_LOADSTRING 100

#pragma region Vari�veis Globais
BOOL(*StartBuff)(void);
BOOL(*GetMSG)(pCommand);
BOOL(*OpenGame)(void);
BOOL(*setGame)(pJogo game);
BOOL(*wrtMSG)(Command);
pBuff(*rbuff)();
HMODULE hDLL;
HANDLE hThreadListener, eGameUpdate, mGameAcess, hThreadGame, eGameStart;
HANDLE hThreadNaveEsquiva, hThreadNaveBasica, hThreadNaveBombas, hThreadTiros;
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

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
bool Alcool(Player *p);
//UI FUNCS
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Dif(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#pragma endregion

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE hPrevInstance,_In_ LPWSTR    lpCmdLine,_In_ int       nCmdShow)
{

	//DWORD threadId[]; //Id da thread a ser criada
	//HANDLE hT[4] = (HANDLE *)malloc(4 * sizeof(HANDLE)); //HANDLE/ponteiro para a thread a ser criada

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif
	srand((int)time(NULL));

	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDS_SPACEINVADERS, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);


	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

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

	/*_tprintf(TEXT("PARA TESTES DE COMS COM GATEWAY E INICIALIZA��O-> Insira dificuldade (1,2,3)"));
	_tscanf_s(_T("%d"), &resp);
	pGameView->dificuldade = resp;
	_tprintf(TEXT("PARA TESTES DE COMS COM GATEWAY E INICIALIZA��O-> Startgame? (1)"));
	_tscanf_s(_T("%d"), &resp);
	if (resp == 1) {
		start_Jogo();

		SetEvent(eGameStart);//sinaliza gateway de altera��es atravez do evento
		ResetEvent(eGameStart);//fecha a sinaliza��o do evento
		SetEvent(eGameUpdate);//sinaliza gateway de altera��es atravez do evento
		ResetEvent(eGameUpdate);//fecha a sinaliza��o do evento
		hThreadGame = CreateThread(NULL, 0, thread_Jogo, NULL, 0, 0);
		if (hThreadGame == NULL) {
			_tprintf(TEXT("[(DEBUG)Thread:Erro-> Error starting listener thread\n %d  \n"), GetLastError());
			return 0;
		}

	}*/
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));
	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	//WaitForSingleObject(hThreadListener, INFINITE);
	CloseHandle(hThreadListener);
	CloseHandle(hThreadGame);
	CloseHandle(hThreadNaveBasica);
	CloseHandle(hThreadNaveEsquiva);

	
	_tprintf(TEXT("[Thread Principal %d]Vou terminar..."), GetCurrentThreadId());
	
	return (int)msg.wParam;
	return 0;
}

#pragma region GUI FUncs
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDR_MENU);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

	return RegisterClassExW(&wcex);
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HMENU menu = GetMenu(hWnd);
	HWND hList = NULL;
	switch (message)
	{
	case WM_INITDIALOG:
		EnableMenuItem(menu, ID_TERMINATEGAME, MF_GRAYED);

	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

		case ID_NEWGAME: //StartGame

			if (pGameView->nPlayers == 0) {
			
				MessageBox(NULL, _T("No players connected."), _T("Unnable to start"), MB_OK | MB_ICONERROR| MB_TASKMODAL);

			
			}
			else {
			hList = CreateWindowEx(0, _T("listbox"), _T("LOG"), WS_CHILD | WS_VISIBLE | LBS_STANDARD | WS_CAPTION | LBS_HASSTRINGS, 0, 0, 400, 100, hWnd, (HMENU)1, NULL, 0);
				EnableMenuItem(menu, ID_NEWGAME, MF_GRAYED);
				EnableMenuItem(menu, ID_TERMINATEGAME, MF_ENABLED);
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DIFF), hWnd, Dif);
			}
			break;
		case ID_TERMINATEGAME: //TermGame
			EnableMenuItem(menu, ID_TERMINATEGAME, MF_GRAYED);
			EnableMenuItem(menu, ID_NEWGAME, MF_ENABLED);
			ShowWindow(hList, SW_HIDE);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Dif(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		CheckRadioButton(hDlg, IDC_RADIO1, IDC_RADIO3, IDC_RADIO1);
		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BOMBFREQ), WM_USER + 5, (WPARAM)TRUE, (LPARAM)30);

		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_VELINV), WM_USER + 5, (WPARAM)TRUE, (LPARAM)50);

		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_POWERFREQ), WM_USER + 5, (WPARAM)TRUE, (LPARAM)20);

		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_POWERDUR), WM_USER + 5, (WPARAM)TRUE, (LPARAM)40);

		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_PLAYERLIFES), WM_USER + 5, (WPARAM)TRUE, (LPARAM)20);

		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_VELBOMB), WM_USER + 5, (WPARAM)TRUE, (LPARAM)60);

		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_VELTIRO), WM_USER + 5, (WPARAM)TRUE, (LPARAM)60);

	case WM_COMMAND:


		if (LOWORD(wParam) == IDOK)
		{
			if (IsDlgButtonChecked(hDlg, IDC_RADIO1)) {
				pGameView->dificuldade = 1;
			}
			else if (IsDlgButtonChecked(hDlg, IDC_RADIO2)) {
				pGameView->dificuldade = 2;
			}
			else {
				pGameView->dificuldade = 3;
			}
			result = SendMessage(GetDlgItem(hDlg, IDC_SLIDER_VELINV), WM_USER, (WPARAM)0, 0);
			pGameView->velNave = result;
			result = SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BOMBFREQ), WM_USER, (WPARAM)0, 0);
			pGameView->fBombas = result;
			result = SendMessage(GetDlgItem(hDlg, IDC_SLIDER_POWERFREQ), WM_USER, (WPARAM)0, 0);
			pGameView->pPower = result;
			result = SendMessage(GetDlgItem(hDlg, IDC_SLIDER_POWERDUR), WM_USER, (WPARAM)0, 0);
			pGameView->DurPower = result;
			result = SendMessage(GetDlgItem(hDlg, IDC_SLIDER_PLAYERLIFES), WM_USER, (WPARAM)0, 0);
			pGameView->nVidasPlayer = result/10;
			result = SendMessage(GetDlgItem(hDlg, IDC_SLIDER_VELBOMB), WM_USER, (WPARAM)0, 0);
			pGameView->velPoweupBomba = result;
			result = SendMessage(GetDlgItem(hDlg, IDC_SLIDER_VELTIRO), WM_USER, (WPARAM)0, 0);
			pGameView->velTiro = result;
	
			start_Jogo();

			SetEvent(eGameStart);//sinaliza gateway de altera��es atravez do evento
			ResetEvent(eGameStart);//fecha a sinaliza��o do evento
			SetEvent(eGameUpdate);//sinaliza gateway de altera��es atravez do evento
			ResetEvent(eGameUpdate);//fecha a sinaliza��o do evento
			hThreadGame = CreateThread(NULL, 0, thread_Jogo, NULL, 0, 0);
			if (hThreadGame == NULL) {
				_tprintf(TEXT("[(DEBUG)Thread:Erro-> Error starting listener thread\n %d  \n"), GetLastError());
				return 0;
			}
			EndDialog(hDlg, LOWORD(wParam));

			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)FALSE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
#pragma endregion
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
	int x = 0;
	int y = 0;

	switch (temp.cmd) {
	case 0:

		pGameView->nPlayers++;
		pGameView->player[temp.id].id = temp.id;
		CopyMemory(pGameView->player[temp.id].username,temp.username, sizeof(temp.username));
		break;
	case LEFT:
		if (Alcool(&pGameView->player[temp.id]))
			x += 1;
		else x -= 1;
		break;
	case RIGHT:
		if (Alcool(&pGameView->player[temp.id]))
			x -= 1;
		else x += 1;
		break;
	case UP:
		if (Alcool(&pGameView->player[temp.id]))
			y += 1;
		else y -= 1;
		break;
	case DOWN:
		if (Alcool(&pGameView->player[temp.id]))
			y -= 1;
		else y += 1;
		break;
	case SHOT: //dispara tiro
		shot(&pGameView->player[temp.id]);
		break;
	default:
		_tprintf(TEXT("\n[TrataComando] Leitura: %d %d %hs\n"), temp.id, temp.cmd, temp.username);

		break;
	}
	movePlayer(&pGameView->player[temp.id], x, y);

//	SetEvent(eGameUpdate);//sinaliza gateway de altera��es atravez do evento
	//ResetEvent(eGameUpdate);//fecha a sinaliza��o do evento

	ReleaseMutex(mGameAcess); //liberta mutex
	
}

bool Alcool(Player *p) {
	return false;
	for (int i = 0; i < 10; i++) {
		if (p->powerups[i].e.id[0] == 'A')
			return true;
	}
	return false;
}

void movePlayer(Player *p, int x, int y) {
	int xl = x + p->nave.e.largura - 1;
	int ya = y + p->nave.e.altura  - 1;
	
	if ((p->nave.e.y + ya) <= 0 || (p->nave.e.y + y) <= pGameView->maxY*0.8)return;
	if (p->nave.e.x +xl >= pGameView->maxX || p->nave.e.x+x <0|| p->nave.e.y +ya >= pGameView->maxY || p->nave.e.y+ya<=0) return;

	for (int i = 0; i < pGameView->nPlayers; i++) {
		if (p->id != pGameView->player[i].id) {
			int x2 = pGameView->player[i].nave.e.x;
			int x2l = x2 + pGameView->player[i].nave.e.largura - 1;
			int y2 = pGameView->player[i].nave.e.y;
			int y2a = y2 + pGameView->player[i].nave.e.altura - 1;
			if ((y >= y2 && y <= y2a) || (ya >= y2 && ya <= y2a) || (y2 >= y && y2 <= ya) || (y2a >= y && y2a <= ya))
				if ((x >= x2 && x <= x2l) || (xl >= x2 && xl <= x2l) || (x2 >= x && x2 <= xl) || (x2l >= x && x2l <= xl))
					return;
		}
	}
	p->nave.e.x += x;
	p->nave.e.y += y;
	/*
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
	pGameView->maxY = 70;

	
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
		pGameView->navesnormais[i].i_desparo = rand() % pGameView->fBombas;
		
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
		pGameView->navesesquivas[i].vida = 3;
	}

	for (int i = pGameView->nNavesEsquivas; i < 30; i++)
		pGameView->navesesquivas[i].vida = 0;
#pragma endregion

#pragma region Tiro


	for (int i = 0; i < 100; i++) {
		pGameView->tiros[i].e.id[0] = 'i';
	}

#pragma endregion

#pragma region Bombas

	for (int i = 0; i < 20; i++) {
		pGameView->bombas[i].e.id[0] = 'i';
	}

#pragma endregion
	
#pragma region PowerUps

	for (int i = 0; i < 20; i++) {
		pGameView->powerups[i].e.id[0] = 'i';
	}

#pragma endregion

#pragma region Player

	for (int i = 0; i < pGameView->nPlayers; i++) {
		pGameView->player[i].nave.vida = 1;
		pGameView->player[i].nvidas = pGameView->nVidasPlayer;
		pGameView->player[i].nave.e.altura = 3;
		pGameView->player[i].nave.e.largura = 3;
		pGameView->player[i].nave.e.id[0] = 'P';
		pGameView->player[i].nave.e.id[1] = (i + 1) / 10 + '0';
		pGameView->player[i].nave.e.id[2] = (i + 1) % 10 + '0';
	
		pGameView->player[i].nave.e.y = pGameView->maxY-pGameView->player[i].nave.e.altura; // (int)pGameView->maxY *0.8;
		pGameView->player[i].nave.e.x = pGameView->maxX / 2 + (i*4);
	}

	for (int i = pGameView->nPlayers; i < 5; i++)
		pGameView->player[i].nave.vida = 0;
#pragma endregion

	ReleaseMutex(mGameAcess);
	
	//TODO-> TRATAMENTO DE JOGO, POWERUPS E TIROS ATACANTES

	return;
}
#pragma endregion

#pragma region Threads
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

DWORD WINAPI thread_basica(LPVOID nave) {

	while (pGameView->nNavesNormais > 0) {

		Sleep( (DWORD)(1000 - pGameView->velNave*10));
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

		/*
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


	


		for (int j = 0; j < 50; j++) {
			for (int i = 0; i < 51; i++) {
				_tprintf(TEXT("%c"), Pmapa[i][j]);
			}
			_tprintf(TEXT("\n"));
		}
		#pragma endregion
		*/

		ReleaseMutex(mGameAcess);
		SetEvent(eGameUpdate);		//sinaliza gateway de altera��es atravez do evento
									//Sleep(100);
		ResetEvent(eGameUpdate);	//fecha a sinaliza��o do evento
	}

	return 0;
}

DWORD WINAPI thread_esquiva(LPVOID nave) {
	//	Nave *n = gameData.navesesquivas;

	while (pGameView->nNavesEsquivas > 0) {
		Sleep((DWORD)(1000 - (pGameView->velNave * 10)*0.60));
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
		SetEvent(eGameUpdate);		//sinaliza gateway de altera��es atravez do evento
									//Sleep(100);
		ResetEvent(eGameUpdate);	//fecha a sinaliza��o do evento
	}
	return 0;
}

DWORD WINAPI thread_tiros(LPVOID data) {
	while (1) {
		Sleep((DWORD)(1000 - (pGameView->velTiro * 10)));
		WaitForSingleObject(mGameAcess,INFINITE);
		for (int i = 0; i < 100; i++) {
			if ((pGameView->tiros[i].e.id[0] != 'i')) {
				if (pGameView->tiros[i].e.y < 0)
					pGameView->tiros[i].e.id[0] = 'i';
				else {
					pGameView->tiros[i].e.y -= 1;
					verifyColision(&pGameView->tiros[i]);
				}
			}
		}
		ReleaseMutex(mGameAcess);
		SetEvent(eGameUpdate);		//sinaliza gateway de altera��es atravez do evento
									
		ResetEvent(eGameUpdate);	//fecha a sinaliza��o do evento
	}
	return 0;
}

DWORD WINAPI thread_bombas(LPVOID data) {
	int random;
	srand((int)time(NULL));
	while (1) {
		Sleep((DWORD)(1000 - (pGameView->velPoweupBomba * 10)));
		WaitForSingleObject(mGameAcess, INFINITE);

#pragma region Bombas

		for (int i = 0; i < 20; i++) {
			if ((pGameView->bombas[i].e.id[0] == 'b')) {
				if (pGameView->bombas[i].e.y > pGameView->maxY)
					pGameView->bombas[i].e.id[0] = 'i';
				else
					pGameView->bombas[i].e.y += 1;
			}
		}

#pragma endregion

#pragma region PowerUps

		random = rand() % 100;
		for (int i = 0; i < 20; i++) {
			if ((pGameView->powerups[i].e.id[0] != 'i')) {
				if (pGameView->powerups[i].e.y > pGameView->maxY)
					pGameView->powerups[i].e.id[0] = 'i';
				else
					pGameView->powerups[i].e.y += 1;
			}
			else {
				
				if (random < pGameView->pPower) {
					random = rand() % 10;
					if (random == 0) {
						pGameView->powerups[i].e.id[0] = 'V';
					}
					else if (random <= 2) {
						random = rand() % 3;
						if (random == 0)
							pGameView->powerups[i].e.id[0] = 'G';
						else if (random == 1)
							pGameView->powerups[i].e.id[0] = 'B';
						else if (random == 2)
							pGameView->powerups[i].e.id[0] = 'A';
					}
					else {
						random = rand() % 2;
						if (random == 0)
							pGameView->powerups[i].e.id[0] = 'E';
						else if (random == 1)
							pGameView->powerups[i].e.id[0] = 'M';
					}
					pGameView->powerups[i].e.x = rand() % pGameView->maxX-1;
					pGameView->powerups[i].e.y = 0;
					random=101;
				}
			}
		}

#pragma endregion
		
		ReleaseMutex(mGameAcess);
		SetEvent(eGameUpdate);		//sinaliza gateway de altera��es atravez do evento
									//Sleep(100);
		ResetEvent(eGameUpdate);	//fecha a sinaliza��o do evento
	}
	return 0;
}
#pragma endregion

#pragma region Naves
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
		l = pGameView->navesnormais[i].e.largura - 1;
		a = pGameView->navesnormais[i].e.altura - 1;
		if (pGameView->navesnormais[i].vida > 0) {
			if (t->e.y >= y && t->e.y <= y + a) {
				if (t->e.x >= x && t->e.x <= x + l) {
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
