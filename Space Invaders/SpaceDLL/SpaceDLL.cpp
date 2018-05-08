#include "SpaceDLL.h"


//A INICIAR COM O SERVER
BOOL startSpaceServer(Jogo jogo)
{
	//criação de mutex de acesso ao jogo
	mGame = CreateMutex(NULL, FALSE, _T("mutexGAME"));
	if (mGame == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> MUTEX GAME\n"));
		return FALSE;
	}

	//criação de espaço de memoria partilhada para o jogo
	hGame = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Jogo), _T("jogo"));
	if (mGame == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> FILEMAPPING GAME\n"));
		return FALSE;
	}
	//vista da memoria partilhada
	pGameView = (pJogo)MapViewOfFile(hGame, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Jogo));
	if (pGameView == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> FILEVIEW GAME\n"));
		return FALSE;
	}

	//BUFFER -> criação de handles
	//mutex de acesso
	mBuffer = CreateMutex(NULL, FALSE, _T("mBuff"));
	if (mBuffer == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> CMUTEX BUFFER\n"));
		return FALSE;
	}

	//semaforo para escrita, items todos disponiveis
	sBuffR = CreateSemaphore(NULL,BUFFSIZE, BUFFSIZE,_T("sBuffR"));
	if (sBuffR == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> C SEM READ BUFFER\n"));
		return FALSE;
	}
	//semaforo para leitura, vazio
	sBuffW = CreateSemaphore(NULL, 0, BUFFSIZE, _T("sBuffW"));
	if (sBuffW == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> C SEM WRITE BUFFER\n"));
		return FALSE;
	}

	return TRUE;
}

//A INICIAR COM O GATEWAY
BOOL openGameMemory()
{
	//abertura do mutex de acesso ao jogo
	mGame = OpenMutex(MUTEX_ALL_ACCESS, FALSE, _T("mutexGAME"));
	if (mGame == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> O MUTEX GAME\n"));
		return FALSE;
	}
	
	hGame = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("jogo"));
	if (mGame == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> FILEMAPPING OPEN\n"));
		return FALSE;
	}

	pGameView = (pJogo)MapViewOfFile(hGame, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Jogo));
	if (mGame == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> FILEVIEW OPEN\n"));
		return FALSE;
	}

	//BUFFER -> abertura de handles
	//mutex de acesso
	mBuffer = OpenMutex(MUTEX_ALL_ACCESS, FALSE, _T("mBuff"));
	if (mBuffer == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> O MUTEX BUFFER\n"));
		return FALSE;
	}

	//semaforo para escrita, items
	sBuffR = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, _T("sBuffR"));
	if (sBuffR == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> O SEM READ BUFFER\n"));
		return FALSE;
	}
	//semaforo para leitura, vazio
	sBuffW = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, _T("sBuffW"));
	if (sBuffW == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> O SEM WRITE BUFFER\n"));
		return FALSE;
	}



	return TRUE;
}

//Setter de dados na memoria partilhada respeitantes ao jogo
BOOL setGame(pJogo game)
{
	if (game != NULL && pGameView != NULL)
	{
		WaitForSingleObject(mGame, INFINITE);
		CopyMemory(pGameView, game, sizeof(Jogo));
		ReleaseMutex(mGame);
		return TRUE;
	}
	return FALSE;
}

//Getter de dados na memoria partilhada respeitantes ao jogo
BOOL getGame(pJogo game)
{
	if (game != NULL && pGameView != NULL)
	{
		WaitForSingleObject(mGame, INFINITE);
		CopyMemory(game, pGameView, sizeof(Jogo));
		ReleaseMutex(mGame);
		return TRUE;
	}
	return FALSE;
}





