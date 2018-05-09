
#include "stdafx.h"

#include "SpaceDLL.h"


HANDLE hGame, mGame;
pJogo pGameView;
HANDLE hBuffer, mBuffer, sBuffR, sBuffW;
 pBuff pBuffView;



BOOL WINAPI DllMain(HANDLE hInstance, DWORD razao, LPVOID tipo_declaracao) {
	switch (razao) {
	case DLL_PROCESS_ATTACH:
		if (tipo_declaracao == NULL)
			_tprintf(TEXT("O processou carregou a biblioteca explicitamente"));
		else
			_tprintf(TEXT("O processou carregou a biblioteca implicitamente"));
		break;
	case DLL_PROCESS_DETACH:
		if (tipo_declaracao == NULL)
			_tprintf(TEXT("O processou desmapeou a biblioteca explicitamente"));
		else
			_tprintf(TEXT("O processou desmapeou a biblioteca implicitamente"));
		break;
	}
	return 1;
	}




//A INICIAR COM O SERVER
BOOL startSpaceServer()
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
		_tprintf(_T("(DEBUG)DLL:Erro-> C MUTEX BUFFER\n"));
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
	//cria buffer
	hBuffer = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Buff), _T("buffer"));
	if (hBuffer == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> CREATING FILEMAP BUFFER\n"));
		return FALSE;
	}
	//cria mapa de visualização do buffer
	pBuffView = (pBuff)MapViewOfFile(hBuffer, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Buff));
	if (pBuffView == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> CREATING MAP VIEW BUFFER\n"));
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
	if (hGame == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> FILEMAPPING OPEN\n"));
		return FALSE;
	}

	pGameView = (pJogo)MapViewOfFile(hGame, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Jogo));
	if (pGameView == NULL) {
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


//cria o buffer ->TODO verificações
BOOL startBuffer() //a ser chamado no servidor cria o buffer NÃO ESTA A INICIALIZAR INDEX A 0
{
	Jogada j; j.id = 0; j.Dir = 0;
	for (int i=0; i < BUFFSIZE; i++) {
		pBuffView->buffer[i] = j;
	}

	pBuffView->nextIn = 0;  //ESTA MERDA NAO TA A FUNCIONAR NAO SEI PORQUEEEE!!!!
	pBuffView->nextOut = 0;

	_tprintf(TEXT("INDEX BUFFER INICIAL %d \n", pBuffView->nextIn));
	return TRUE;
}



BOOL openBuffer() //a ser chamado no gateway
{
	hBuffer = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("buffer"));;
	if (hBuffer == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> OPEN FILEMAP BUFFER\n"));
		return FALSE;
	}
	pBuffView = (pBuff)MapViewOfFile(hBuffer, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Buff));
	if (pBuffView == NULL) {
		_tprintf(_T("(DEBUG)DLL:Erro-> OPEN MAP VIEW BUFFER\n"));
		return FALSE;
	}

	return TRUE;
}

BOOL WriteBuffer(Jogada jogada)
{
	_tprintf(TEXT("VOU ESCREVER NO BUFFER \n"));

	int index;
	//aguarda posição vazia no semaforo da escrita
	WaitForSingleObject(sBuffR, INFINITE);
	//aguarda o mutex de acesso ao buffer
	WaitForSingleObject(mBuffer, INFINITE);
	
	pBuffView->buffer[pBuffView->nextIn] = jogada; _tprintf(TEXT("INDEX BUFFER %d \n", pBuffView->nextIn));
	pBuffView->nextIn++;// = (pBuffView->nextIn) % BUFFSIZE; 
	/* //METODO ALTERNATIVO DE ESCRITA
	index = pBuffView->nextIn; _tprintf(TEXT("INDEX BUFFER %d \n", index));
	//copia a jogada recebida para a posição de entrada no buffer
	CopyMemory(&pBuffView->buffer[index], jogada, sizeof(Jogada));
	//define nova posição de entrada e copia-a para a estrutura na SHM
	index= (index +1) % BUFFSIZE;
	_tprintf(TEXT("INDEX BUFFER proximo %d\n", index));
	CopyMemory(&pBuffView->nextIn, &index, sizeof(int));
	*/
	ReleaseMutex(mBuffer); //liberta mutex de buffer
	ReleaseSemaphore(sBuffW,1,NULL); //liberta uma posição no semaforo de leituras


	return TRUE;
}

pBuff returnBuff() {
	
	return pBuffView;
}

Jogada ReadBuffer()
{
	_tprintf(TEXT("VOU LER NO BUFFER \n"));
	int index;
	//aguarda posição vazia no semaforo da Leitura
	WaitForSingleObject(sBuffW, INFINITE);
	//aguarda o mutex de acesso ao buffer
	WaitForSingleObject(mBuffer, INFINITE);
	Jogada jogada;
	jogada = pBuffView->buffer[pBuffView->nextOut];
	pBuffView->nextOut = (pBuffView->nextOut + 1) % BUFFSIZE;
	

	/*int zero = 0;
	index = pBuffView->nextOut;
	//copia a jogada recebida para a posição de entrada no buffer
	CopyMemory(jogada, &pBuffView->buffer[index],sizeof(Jogada));
	//define nova posição de entrada e copia-a para a estrutura na SHM
	index = (index + 1) % BUFFSIZE;
	CopyMemory(&pBuffView->nextIn, &index, sizeof(int));*/

	ReleaseMutex(mBuffer); //liberta mutex de buffer
	ReleaseSemaphore(sBuffR, 1, NULL); //liberta uma posição no semaforo de escritas


	return jogada;
}
