#include "../Space Invaders/utils.h"


#define PIPE_NAME TEXT("\\\\.\\pipe\\GamePipe")

//VARIAVEIS GLOBAIS
HANDLE hPipe;
TCHAR szProgName[] = TEXT("SpaceInvaders Client");
//THREADS
DWORD WINAPI thread_read(LPVOID data);
LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);

//FUNCS
BOOL thread_write(Command c);

//GlobalVars
int idplayer;

//MAIN
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	int i = 0;
	MSG lpMsg;
	HWND hWnd; // hWnd é o handler da janela, gerado mais abaixo por CreateWindow()
	WNDCLASSEX wcApp; // WNDCLASSEX é uma estrutura cujos membros servem para definir as características da classe da janela

	wcApp.cbSize = sizeof(WNDCLASSEX); // Tamanho da estrutura WNDCLASSEX
	wcApp.hInstance = hInst; // Instância da janela actualmente exibida


	wcApp.lpszClassName = szProgName; // Nome da janela (neste caso = nome do programa)
	wcApp.lpfnWndProc = TrataEventos; // Endereço da função de processamento da janela


	wcApp.style = CS_HREDRAW | CS_VREDRAW;// Estilo da janela: Fazer o redraw se for
										  // modificada horizontal ou verticalmente
	wcApp.hIcon = LoadIcon(NULL, IDI_INFORMATION);// "hIcon" = handler do ícon normal
												  //"NULL" = Icon definido no Windows
												  // "IDI_AP..." Ícone "aplicação"
	wcApp.hIconSm = LoadIcon(NULL, IDI_ERROR);// "hIconSm" = handler do ícon pequeno
											  //"NULL" = Icon definido no Windows
											  // "IDI_INF..." Ícon de informação
	wcApp.hCursor = LoadCursor(NULL, IDC_ARROW); // "hCursor" = handler do cursor (rato)
												 // "NULL" = Forma definida no Windows
												 // "IDC_ARROW" Aspecto "seta"
	wcApp.lpszMenuName = MAKEINTRESOURCE(IDC_HELP); // Classe do menu que a janela pode ter
													// (NULL = não tem menu)
	wcApp.cbClsExtra = 0; // Livre, para uso particular
	wcApp.cbWndExtra = 0; // Livre, para uso particular
	wcApp.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	// "hbrBackground" = handler para "brush" de pintura do fundo da janela. Devolvido por
	// "GetStockObject".Neste caso o fundo será branco


	if (!RegisterClassEx(&wcApp))//  Registar a classe "wcApp" no Windows
		return(0);

	hWnd = CreateWindow(
		szProgName, // Nome da janela (programa) definido acima
		TEXT("Coisas lindas em SO2"),// Texto que figura na barra do título
		WS_OVERLAPPEDWINDOW, // Estilo da janela (WS_OVERLAPPED= normal)
		100, // Posição x pixels (default=à direita da última)
		100, // Posição y pixels (default=abaixo da última)
		300, // Largura da janela (em pixels)
		300, // Altura da janela (em pixels)
		(HWND)HWND_DESKTOP, // handle da janela pai (se se criar uma a partir de
							// outra) ou HWND_DESKTOP se a janela for a primeira,
							// criada a partir do "desktop"
		(HMENU)NULL, // handle do menu da janela (se tiver menu)
		(HINSTANCE)hInst, // handle da instância do programa actual ("hInst" é
						  // passado num dos parâmetros de WinMain()
		0); // Não há parâmetros adicionais para a janela


	ShowWindow(hWnd, nCmdShow); // "hWnd"= handler da janela, devolvido por
	UpdateWindow(hWnd); // Refrescar a janela (Windows envia à janela uma



	//PIPING
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
	//FINALIZA PIPES

	//INICIA THREADS
	HANDLE htTreadr;
	htTreadr = CreateThread(NULL, 0, thread_read, (LPVOID)hWnd, 0, NULL);
	//TERMINA THREADS



	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg); // Pré-processamento da mensagem (p.e. obter código
								  // ASCII da tecla premida)
		DispatchMessage(&lpMsg); // Enviar a mensagem traduzida de volta ao Windows, que
								 // aguarda até que a possa reenviar à função de
								 // tratamento da janela, CALLBACK TrataEventos (abaixo)
		UpdateWindow(hWnd);
	}

	//WaitForSingleObject(htTreadr, INFINITE);

	CloseHandle(htTreadr);
	CloseHandle(hPipe);

	Sleep(200);
	return((int)lpMsg.wParam); // Retorna sempre o parâmetro wParam da estrutura lpMsg
}

void start() {










}


DWORD WINAPI thread_read(LPVOID data) {
	message msg;
	BOOL ret=false;
	DWORD n;
	HANDLE IOReady;
	OVERLAPPED Ov;
	BOOL firstrun=true;
	HWND hWnd = (HWND)data;
	HDC hdc;
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
			
			
			if (firstrun) {
				idplayer = msg.idPlayer;
				firstrun = false;
			}
			_tprintf(TEXT("[DEBUG] Recebi %d bytes, idplayer: %d\n"), n, msg.idPlayer);
			InvalidateRect(hWnd, NULL, TRUE);
			//TESTE APRESENTA MAPA
			PAINTSTRUCT ps;
			hdc = BeginPaint(hWnd, &ps);


			for (int i = 0; i < msg.jogo.nNavesNormais; i++) {
					Rectangle(hdc, msg.jogo.navesnormais[i].e.x, msg.jogo.navesnormais[i].e.y, msg.jogo.navesnormais[i].e.x + 5, msg.jogo.navesnormais[i].e.y + 5);
					//TextOut(hdc, x + 5, y + 5, letras, 1);
			}
			EndPaint(hWnd, &ps);



			//FIM TESTE APRESENTA MAPA
		}
	}
	return 0;
}

BOOL thread_write(Command c) {
	DWORD n;
	BOOL ret;
	//overlaped I/O
	HANDLE IOReady;
	OVERLAPPED Ov;

	
	IOReady = CreateEvent(NULL, TRUE, FALSE, NULL);

		ZeroMemory(&Ov, sizeof(Ov));
		ResetEvent(IOReady);
		Ov.hEvent = IOReady;

		WriteFile(hPipe, &c, sizeof(Command), &n, &Ov);

		WaitForSingleObject(IOReady, INFINITE);
		ret = GetOverlappedResult(hPipe, &Ov, &n, FALSE);

		if (!ret) {
			_tprintf(TEXT("[ERRO] Escrever no pipe!\n"));
			return false;
		}
		_tprintf(TEXT("[DEBUG] Enviei %d bytes ao gateway...\n"), n);
		return true;



	return 0;
}

LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {

	static TCHAR letras[10] = TEXT("/0");
	static int x, y;
	HDC hdc;
	switch (messg) {


	case WM_DESTROY: // Destruir a janela e terminar o programa
		int ret;		 // "PostQuitMessage(Exit Status)"
		PostQuitMessage(0);
		break;

	case TPM_LEFTBUTTON:
		break;
	case WM_CHAR:
		letras[0] = wParam;
		break;
	case WM_LBUTTONDOWN:
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		InvalidateRect(hWnd, NULL, TRUE);

		break;

	case WM_PAINT:

		break;
	default:
		// Neste exemplo, para qualquer outra mensagem (p.e. "minimizar","maximizar","restaurar")
		// não é efectuado nenhum processamento, apenas se segue o "default" do Windows
		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break;
	}
	return(0);
}
