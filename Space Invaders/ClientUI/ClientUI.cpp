// ClientUI.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "ClientUI.h"

#include "../Space Invaders/utils.h"

#define MAX_LOADSTRING 100
#define PIPE_NAME TEXT("\\\\.\\pipe\\GamePipe")

// Global Variables:
HINSTANCE hInst;                                // current instance
TCHAR szTitle[MAX_LOADSTRING] = TEXT("SpaceInvaders Client");                  // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING] = TEXT("SpaceInvaders Client");;            // the main window class name
HANDLE hPipe;
char username[20];
int PlayerID;
BOOL Login();
HWND hWndp;

int windowMode=0;

//THREADS
DWORD WINAPI thread_read(LPVOID data);

//FUNCS
//BOOL write_command(Command c);
BOOL CALLBACK DialogAuthenticate(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
BOOL write_command(int i);


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
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInst, nullptr);

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
    switch (message)
    {
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
			break;
		}
	}
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


BOOL CALLBACK DialogAuthenticate(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	switch (messg) {

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


			_tprintf(TEXT("[DEBUG] Esperar pelo pipe '%s' (WaitNamedPipe)\n"), PIPE_NAME);
			if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
				_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'!\n"), PIPE_NAME);
				MessageBox(NULL, _T("No connection"), _T("No Connection, please connect the server."), MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
			
				PostQuitMessage(0);

				CloseWindow(hWnd);
				return 0;
			}
			_tprintf(TEXT("[DEBUG] Ligação ao pipe do escritor... (CreateFile)\n"));
			hPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0 | FILE_FLAG_OVERLAPPED, NULL);
			if (hPipe == NULL) {
				_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), PIPE_NAME);
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
	HDC hdc;
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
			InvalidateRect(hWndp, NULL, TRUE);
			//TESTE APRESENTA MAPA
			PAINTSTRUCT ps;
			hdc = BeginPaint(hWndp, &ps);


			for (int i = 0; i < msg.jogo.nNavesNormais; i++) {
				Rectangle(hdc, msg.jogo.navesnormais[i].e.x, msg.jogo.navesnormais[i].e.y, msg.jogo.navesnormais[i].e.x + 5, msg.jogo.navesnormais[i].e.y + 5);
				//TextOut(hdc, x + 5, y + 5, letras, 1);
			}
			EndPaint(hWndp, &ps);

		}
	}
	return 0;
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
