// ClientUI.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ClientUI.h"
#include "resource.h"

#include "../Space Invaders/utils.h"

#define MAX_LOADSTRING 100


#pragma region Global Variables
Jogo localGame;
BOOL gameStarted=FALSE;
HINSTANCE hInst;                                // current instance
TCHAR szTitle[MAX_LOADSTRING] = TEXT("SpaceInvaders Client");                  // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING] = TEXT("SpaceInvaders Client");;            // the main window class name
HANDLE hPipe;
char username[20];
int PlayerID;
HWND hWndp;
TCHAR keyLeft = TEXT('A');
TCHAR keyRight = TEXT('D');
TCHAR keyUp = TEXT('W');
TCHAR keyDown = TEXT('S');
TCHAR keyFire = TEXT('F');
int windowMode=0;
HBRUSH hbrush;
TCHAR domain[20];

#pragma region HBIPMAP
HBITMAP hPlayer;
HBITMAP hNaveNormal;
HBITMAP hNaveEsquiva;
HBITMAP hTiro;
HBITMAP hBomba;
#pragma region PowerUps
//Down
HBITMAP hPup_Down_GELO;
HBITMAP	hPup_Down_ESCUDO;
HBITMAP	hPup_Down_Bateria;
HBITMAP	hPup_Down_Vida;
HBITMAP	hPup_Down_Mais;
HBITMAP	hPup_Down_Alcool;
HBITMAP	hPup_Down_Algo;//SuperTiro?
//Active
HBITMAP hPup_Active_GELO;
HBITMAP hPup_Active_ESCUDO;
HBITMAP hPup_Active_Bateria;
HBITMAP hPup_Active_Vida;
HBITMAP hPup_Active_Mais;
HBITMAP hPup_Active_Alcool;
HBITMAP hPup_Active_Algo;
#pragma endregion
#pragma endregion

HDC hdc;
HDC auxDC;
HBITMAP auxBM = NULL;
#pragma endregion

//THREADS
DWORD WINAPI thread_read(LPVOID data);

//FUNCS
void refreshGame(Jogo jogo);
BOOL Login();
BOOL CALLBACK DialogAuthenticate(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
BOOL write_command(int i);
void loadPointsFromRegistry();



// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                CreateMainWindow(HINSTANCE);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,_In_ LPWSTR    lpCmdLine,_In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
	hInst = hInstance;
	windowMode = nCmdShow;
    // Initialize global strings
    LoadStringW(hInst, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInst, IDC_CLIENTUI, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInst);

	hWndp = NULL;
	DialogBox(hInst, MAKEINTRESOURCE(IDD_LOGIN), hWndp, (DLGPROC)DialogAuthenticate);


	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENTUI));
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



	//CloseHandle(htTreadr);
	//CloseHandle(hPipe);

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIENTUI));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CLIENTUI);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL CreateMainWindow(HINSTANCE) {

	hWndp = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 520, 760, nullptr, nullptr, hInst, nullptr);

	if (!hWndp)
	{
		return FALSE;
	}

	ShowWindow(hWndp, windowMode);
	UpdateWindow(hWndp);
	return TRUE;
}

	


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int nX, nY;
	static BITMAP bm;
	static HDC hdcMem;
	static HBITMAP hbmOld;
	PAINTSTRUCT ps;

    switch (message) 
    {

	case WM_CREATE:

		hNaveNormal		   =(HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_NORMAL), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hNaveEsquiva	   =(HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_ESQUIVA), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hPlayer			   =(HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_PLAYER), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hTiro              =(HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_TIRO), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hBomba			   =(HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BOMB), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hPup_Down_GELO	   =(HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_DOWN_ICE), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hPup_Down_ESCUDO   =(HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_DOWN_SHEILD), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hPup_Down_Bateria  =(HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_DOWN_BATTERY), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hPup_Down_Vida	   =(HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_DOWN_LIFE), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hPup_Down_Mais	   =(HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_DOWN_MAIS), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hPup_Down_Alcool   =(HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_DOWN_ALCOOL), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hPup_Down_Algo	   =(HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_DOWN_SHEILD), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hPup_Active_GELO   =(HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hPup_Active_ESCUDO =(HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hPup_Active_Bateria=(HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hPup_Active_Vida   =(HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hPup_Active_Mais   =(HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hPup_Active_Alcool =(HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		hPup_Active_Algo   =(HBITMAP)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);
		
		hdc = GetDC(hWnd);
		GetObject(hNaveNormal, sizeof(bm), &bm);
		GetObject(hNaveEsquiva, sizeof(bm), &bm);
		GetObject(hPlayer, sizeof(bm), &bm);
		GetObject(hTiro, sizeof(bm), &bm);
		GetObject(hBomba, sizeof(bm), &bm);
		GetObject(hPup_Down_GELO, sizeof(bm), &bm);
		GetObject(hPup_Down_ESCUDO, sizeof(bm), &bm);
		GetObject(hPup_Down_Bateria, sizeof(bm), &bm);
		GetObject(hPup_Down_Vida, sizeof(bm), &bm);
		GetObject(hPup_Down_Mais, sizeof(bm), &bm);
		GetObject(hPup_Down_Alcool, sizeof(bm), &bm);
		GetObject(hPup_Down_Algo, sizeof(bm), &bm);
		GetObject(hPup_Active_GELO, sizeof(bm), &bm);
		GetObject(hPup_Active_ESCUDO, sizeof(bm), &bm);
		GetObject(hPup_Active_Bateria, sizeof(bm), &bm);
		GetObject(hPup_Active_Vida, sizeof(bm), &bm);
		GetObject(hPup_Active_Mais, sizeof(bm), &bm);
		GetObject(hPup_Active_Alcool, sizeof(bm), &bm);
		GetObject(hPup_Active_Algo, sizeof(bm), &bm);

		hdcMem = CreateCompatibleDC(hdc);
		ReleaseDC(hWnd, hdc);
	
		//hbrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
		hbrush = CreateSolidBrush(RGB(255, 128, 128));
		nX = GetSystemMetrics(SM_CXSCREEN);
		nY = GetSystemMetrics(SM_CYSCREEN);
		hdc = GetDC(hWnd);

		auxDC = CreateCompatibleDC(hdc);
		auxBM = CreateCompatibleBitmap(hdc, nX, nY);

		SelectObject(auxDC, auxBM);
		SelectObject(auxDC, GetStockObject(BLACK_BRUSH));

		PatBlt(auxDC, 0, 0, nX, nY, PATCOPY);
		ReleaseDC(hWnd, hdc);

#pragma endregion
		break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {

            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;


	case WM_KEYDOWN:
	{

		switch (wParam) {

		case VK_LEFT:
			write_command(LEFT);
			break;

		case VK_RIGHT:
			write_command(RIGHT);
			break;

		case VK_UP:
			write_command(UP);
			break;

		case VK_DOWN:
			write_command(DOWN);
			break;
		case VK_SPACE:
			write_command(SHOT);
			break;
		default:
			if (wParam == keyLeft) {
				write_command(LEFT);
			}
			if (wParam == keyUp) {
				write_command(UP);
			}
			if (wParam == keyRight) {
				write_command(RIGHT);
			}
			if (wParam == keyDown) {
				write_command(DOWN);
			}
			if (wParam == keyFire) {
				write_command(SHOT);
			}
			break;
		}
	}
    case WM_PAINT:
        {

		if(localGame.gameover)
			PostQuitMessage(0);

			PatBlt(auxDC, 0, 0, nX, nY, PATCOPY);
			if (gameStarted) {
				hbmOld = (HBITMAP)SelectObject(hdcMem, hNaveNormal);	
				for (int i = 0; i < localGame.nNavesNormais; i++) {
					if(localGame.navesnormais[i].vida>0)
					StretchBlt(auxDC, (localGame.navesnormais[i].e.x ) * 10, (localGame.navesnormais[i].e.y ) * 10, 30,30, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
				}
				hbmOld = (HBITMAP)SelectObject(hdcMem, hNaveEsquiva);
				for (int i = 0; i < localGame.nNavesEsquivas; i++) {
					if (localGame.navesesquivas[i].vida>0)
					StretchBlt(auxDC, (localGame.navesesquivas[i].e.x ) * 10, (localGame.navesesquivas[i].e.y ) * 10, 30, 30, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

				}
				hbmOld = (HBITMAP)SelectObject(hdcMem, hPlayer);
				for (int i = 0; i < localGame.nPlayers; i++) {
					if (localGame.player[i].nvidas>0)
					StretchBlt(auxDC, (localGame.player[i].nave.e.x ) * 10, (localGame.player[i].nave.e.y ) * 10, 30, 30, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
				}
				hbmOld = (HBITMAP)SelectObject(hdcMem, hTiro);
				for (int i = 0; i < 100; i++) {
					if (localGame.tiros[i].e.id[0]!='i')
						StretchBlt(auxDC, (localGame.tiros[i].e.x) * 10, (localGame.tiros[i].e.y ) * 10, 10, 10, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
				}
				hbmOld = (HBITMAP)SelectObject(hdcMem, hBomba);
				for (int i = 0; i < 20; i++) {
					if (localGame.bombas[i].e.id[0] != 'i')
						StretchBlt(auxDC, (localGame.bombas[i].e.x) * 10, (localGame.bombas[i].e.y) * 10, 10, 10, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
				}
				
				for (int i = 0; i < 20; i++) {
					if (localGame.powerups[i].e.id[0] != 'i') {
						switch (localGame.powerups[i].e.id[0]) {
						case 'V':
							hbmOld = (HBITMAP)SelectObject(hdcMem, hPup_Down_Vida);
							break;
						case 'G':
							hbmOld = (HBITMAP)SelectObject(hdcMem, hPup_Down_GELO);
							break;
						case 'A':
							hbmOld = (HBITMAP)SelectObject(hdcMem, hPup_Down_Alcool);
							break;
						case 'B':
							hbmOld = (HBITMAP)SelectObject(hdcMem, hPup_Down_Bateria);
							break;
						case 'E':
							hbmOld = (HBITMAP)SelectObject(hdcMem, hPup_Down_ESCUDO);
							break;
						case 'M':
							hbmOld = (HBITMAP)SelectObject(hdcMem, hPup_Down_Mais);
							break;
						}
						StretchBlt(auxDC, (localGame.powerups[i].e.x) * 10, (localGame.powerups[i].e.y) * 10, 10, 10, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
					}
				}
			}
			          
			hdc = BeginPaint(hWnd, &ps);
			BitBlt(hdc, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), auxDC, 0, 0, SRCCOPY);
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



BOOL CALLBACK DialogAuthenticate(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	TCHAR buff[100]= _T("\\\\");
	LPTSTR remotePipe;
	TCHAR str[2];
	BOOL log;
	HANDLE hUserToken;

	switch (messg) {
	case WM_INITDIALOG:
		CheckRadioButton(hWnd, IDC_RADIOL, IDC_RADIOR, IDC_RADIOL);

		_stprintf_s(str, TEXT("%c"), _T('A'));
		SendDlgItemMessage(hWnd, IDC_EDIT_LEFT, EM_REPLACESEL, 0, (LPARAM)str);
		_stprintf_s(str, TEXT("%c"), _T('U'));
		SendDlgItemMessage(hWnd, IDC_USERNAME, EM_REPLACESEL, 0, (LPARAM)str);
		_stprintf_s(str, TEXT("%c"), _T('D'));
		SendDlgItemMessage(hWnd, IDC_EDIT_RIGHT, EM_REPLACESEL, 0, (LPARAM)str);
		_stprintf_s(str, TEXT("%c"), _T('W'));
		SendDlgItemMessage(hWnd, IDC_EDIT_TOP, EM_REPLACESEL, 0, (LPARAM)str);
		_stprintf_s(str, TEXT("%c"), _T('S'));
		SendDlgItemMessage(hWnd, IDC_EDIT_DOWN, EM_REPLACESEL, 0, (LPARAM)str);
		_stprintf_s(str, TEXT("%c"), _T('F'));
		SendDlgItemMessage(hWnd, IDC_EDIT_FIRE, EM_REPLACESEL, 0, (LPARAM)str);
		return 1;

	case WM_DESTROY:
		EndDialog(hWnd, 0);
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hWnd, 0);
			PostQuitMessage(0);
			return 0;
			break;
		case IDC_OKLOG:
			TCHAR user[20];
			GetDlgItemText(hWnd, IDC_USERNAME, user, 20);
			//strcpy_s(username, user);

			for (int i = 0; i < 20; i++)
				username[i] = user[i];

			if (IsDlgButtonChecked(hWnd, IDC_RADIOR))
			{

				GetDlgItemText(hWnd, IDC_EDIT_IP, domain, 20);

				
				_tcscat_s(buff,100 ,domain);
				_tcscat_s(buff,100 ,TEXT("\\pipe\\GamePipe"));
				remotePipe = buff;
				log = LogonUser(user, domain, TEXT("SO2"), LOGON32_LOGON_NEW_CREDENTIALS, LOGON32_PROVIDER_DEFAULT, &hUserToken);
				log = ImpersonateLoggedOnUser(hUserToken);

			}
			else {
				remotePipe = (LPTSTR)TEXT("\\\\.\\pipe\\GamePipe");
			}
			GetDlgItemText(hWnd, IDC_EDIT_LEFT, str, 2);
			keyLeft = str[0];
			GetDlgItemText(hWnd, IDC_EDIT_TOP, str, 2);
			keyUp = str[0];
			GetDlgItemText(hWnd, IDC_EDIT_RIGHT, str, 2);
			keyRight = str[0];
			GetDlgItemText(hWnd, IDC_EDIT_DOWN, str, 2);
			keyDown = (str[0]);
			GetDlgItemText(hWnd, IDC_EDIT_FIRE, str, 2);
			keyFire = (str[0]);


			_tprintf(TEXT("[DEBUG] Esperar pelo pipe '%s' (WaitNamedPipe)\n"), remotePipe);
			if (!WaitNamedPipe(remotePipe, NMPWAIT_WAIT_FOREVER)) {
				_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'!\n"), remotePipe);
				MessageBox(NULL, _T("No connection"), _T("No Connection, please connect the server."), MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
			
				PostQuitMessage(0);

				CloseWindow(hWnd);
				return 0;
			}
			_tprintf(TEXT("[DEBUG] Ligação ao pipe do escritor... (CreateFile)\n"));
			hPipe = CreateFile(remotePipe, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0 | FILE_FLAG_OVERLAPPED, NULL);
			if (hPipe == NULL) {
				_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), remotePipe);
				MessageBox(NULL, _T("No connection"), _T("No Connection, please connect the server."), MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);

				PostQuitMessage(0);

				CloseWindow(hWnd);
				return 0;
			}
			_tprintf(TEXT("[DEBUG] Liguei-me...\n"));


			//ALTERAR MODO DO PIPE
			//DWORD modo = PIPE_READMODE_MESSAGE;
			//SetNamedPipeHandleState(hPipe, PIPE_READMODE_MESSAGE, NULL, NULL);
			Login();


			HANDLE htTreadr;
			htTreadr = CreateThread(NULL, 0, thread_read, NULL, 0, NULL);
			if (htTreadr == NULL) {
				_tprintf(TEXT("[ERRO] Inicializar thread! (ThreadLeitura)\n"));
				return 0;
			}
		
			EndDialog(hWnd, 0);
			
			// Perform application initialization:
			if (!CreateMainWindow(hInst)) {
				return FALSE;
			}
		

			break;


		default:
		//	SendDlgItemMessage(hWnd, IDC_USERNAME, CB_ADDSTRING, 0, (LPARAM)_T("USER"));
			break;
		}

	}

	return 0;
}

BOOL Login() {
	DWORD n;
	BOOL ret;
	//overlaped I/O
	HANDLE IOReady;
	OVERLAPPED Ov;

	

	IOReady = CreateEvent(NULL, TRUE, FALSE, NULL);

	ZeroMemory(&Ov, sizeof(Ov));
	ResetEvent(IOReady);
	Ov.hEvent = IOReady;
	//strcpy_s(username,"zeca");
	WriteFile(hPipe, &username, (sizeof(char) * 20), &n, &Ov);

	WaitForSingleObject(IOReady, INFINITE);
	ret = GetOverlappedResult(hPipe, &Ov, &n, FALSE);

	if (!ret) {
		_tprintf(TEXT("[ERRO] Escrever no pipe!\n"));
		return false;
	}
	_tprintf(TEXT("[DEBUG] Enviei %d bytes ao gateway...\n"), n);



	ZeroMemory(&Ov, sizeof(Ov));
	ResetEvent(IOReady);
	Ov.hEvent = IOReady;

	ret = ReadFile(hPipe, &PlayerID, sizeof(int), &n, &Ov);
	WaitForSingleObject(IOReady, INFINITE);
	GetOverlappedResult(hPipe, &Ov, &n, FALSE);

	if (n<sizeof(int)) {
		_tprintf(TEXT("[ERROR] Less bites received %d %d... (Login)\n"), ret, n);
		return FALSE;
	}


	return TRUE;
}

DWORD WINAPI thread_read(LPVOID data) {
	message msg;
	BOOL ret = false;
	DWORD nr;
	HANDLE IOReady;
	OVERLAPPED Ov;

	IOReady = CreateEvent(NULL, TRUE, FALSE, NULL);

	while (1) {
		ZeroMemory(&Ov, sizeof(Ov));
		ResetEvent(IOReady);
		Ov.hEvent = IOReady;

		ret = ReadFile(hPipe, &msg, MSGSIZE, &nr, &Ov);
		WaitForSingleObject(IOReady, INFINITE);
		GetOverlappedResult(hPipe, &Ov, &nr, FALSE);


		/*	if (!ret) {
		_tprintf(TEXT("[ERROR] READING %d %d... threadRead \n"), ret, n);
		continue;
		}*/
		if (nr<MSGSIZE) {
			_tprintf(TEXT("[ERROR] Less bites received %d %d... (ThreadRead)\n"), ret, nr);
			//break;
		}
		else {
			_tprintf(TEXT("[DEBUG] Recebi %d bytes, jogodif: %d\n"), nr, msg.jogo.dificuldade);


			_tprintf(TEXT("[DEBUG] Recebi %d bytes, idplayer: %d\n"), nr, msg.idPlayer);
		
			//TESTE APRESENTA MAPA
			//refreshGame(msg.jogo);
			localGame = msg.jogo;
			if (localGame.gameover)
			{
				if (localGame.whoWins == 1) {


					MessageBox(NULL, _T("You LOOOOSER!!! Invaders win the game. \n "), _T("GAME OVER"), MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
				}
				else if (localGame.whoWins == 2) {

					MessageBox(NULL, _T("Congrats!! Players win the game.\n"), _T("GAME OVER"), MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
				}
				else {
					MessageBox(NULL, _T("Game interrupted by server.\n"), _T("GAME OVER"), MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);

				}
				loadPointsFromRegistry();
				gameStarted = FALSE;
			}
			else{
			gameStarted = TRUE;
			}
			InvalidateRect(hWndp, NULL, FALSE);
		}
	}
	return 0;
}

void refreshGame(Jogo jogo) {
	HDC hdc;
	PAINTSTRUCT ps;
	hdc = BeginPaint(hWndp, &ps);
	


	for (int i = 0; i < jogo.nNavesNormais; i++) {
		Rectangle(hdc, jogo.navesnormais[i].e.x, jogo.navesnormais[i].e.y, jogo.navesnormais[i].e.x + 5, jogo.navesnormais[i].e.y + 5);
	
	}
	for (int i = 0; i < jogo.nNavesEsquivas; i++) {
		Rectangle(hdc, jogo.navesesquivas[i].e.x, jogo.navesesquivas[i].e.y, jogo.navesesquivas[i].e.x + 5, jogo.navesesquivas[i].e.y + 5);
	}

	for (int i = 0; i < jogo.nPlayers; i++) {
		if(jogo.player[i].nvidas>0)
		Rectangle(hdc, jogo.player[i].nave.e.x, jogo.player[i].nave.e.y, jogo.player[i].nave.e.x + 5, jogo.player[i].nave.e.y + 5);
	}

	EndPaint(hWndp, &ps);
	//ReleaseDC(hWndp, hdc);
	InvalidateRect(NULL, NULL, TRUE);
}


BOOL write_command(int i) {
	DWORD n;
	BOOL ret;
	//overlaped I/O
	HANDLE IOReady;
	OVERLAPPED Ov;
	Command c;

	c.id = PlayerID;
	c.cmd = i;
	strcpy_s(c.username, username);
	IOReady = CreateEvent(NULL, TRUE, FALSE, NULL);


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



	return 0;
}

void loadPointsFromRegistry() {
	HKEY regKey;
	DWORD regEvent, tam;
	TCHAR str[200], user[200];
	

	if (RegCreateKeyEx(HKEY_CURRENT_USER,TEXT("Software\\SpaceInvaders"),0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&regKey,&regEvent) != ERROR_SUCCESS) {
		return;
	}
	else {
		if (regEvent == REG_OPENED_EXISTING_KEY) {
			_stprintf_s(user, 200, TEXT("Place:%d"), PlayerID);
			RegQueryValueEx(regKey, user, NULL, NULL, (LPBYTE)str, &tam);
			MessageBox(NULL, str, _T("Points"), MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);

		}
		RegCloseKey(regKey);
	}
}
