#pragma once
#include <time.h>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdbool.h>


//COMANDS
#define LEFT 1
#define RIGHT 2
#define UP 3
#define DOWN 4
#define SHOT 5


#define BUFFSIZE 30
#define PIPEBUFFSIZE 4000
#define TCHARSIZE 30





struct Elemento {
	int x;
	int y;
	int largura;
	int altura;
	char id[3];
};

typedef struct {
	BOOL isLeft;
	BOOL fimJanela;
	int vida;
	int i_desparo;
	Elemento e;
}Nave;

typedef struct {
	TCHAR name;
	int efect;
	int time;
	Elemento e;
}PowerUP;

typedef struct {
	char username[20];
	int id;
	PowerUP powerups[10];
	Nave nave;
	int points;
	int nvidas;
}Player;

typedef struct {
	Elemento e;
}Bomba;
typedef struct {
	Elemento e;
}Tiro;

typedef struct {
	int nNavesNormais;
	int nNormaisVivas;
	int nNavesEsquivas;
	int nEsquivasVivas;
	int nPlayers;
	int velNave;
	int dificuldade;
	int velPoweupBomba; //velocidade das bombas/powerups
	int DurPower; //duracao powerup
	int velTiro; //velocidade de tiros user
	int nVidasPlayer;
	int maxX; 
	int maxY;
	int fBombas;// Frequencia que caí bomba em cada nave
	int pPower;//frequencia powerups
	Tiro tiros[100];
	Bomba bombas[20];
	PowerUP powerups[20];
	Nave navesnormais[30];
	Nave navesesquivas[30];
	Player player[5];
}Jogo, *pJogo;





typedef struct {
	int id;
	int cmd;
	char username[20];
}Command, *pCommand;

typedef struct {
	int nextIn;	//indice de escrita
	int nextOut; //indice de leitura
	Command buffer[BUFFSIZE];
}Buff, *pBuff;

struct message {
	int idPlayer;
	Jogo jogo;
};
#define MSGSIZE sizeof(message)