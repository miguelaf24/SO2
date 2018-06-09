#pragma once
#include <time.h>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdbool.h>




#define BUFFSIZE 30
#define PIPEBUFFSIZE 4000
#define TCHARSIZE 30

//direções
#define LEFT 0
#define UP 1
#define RIGHT 2
#define DOWN 3



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
	double velocidade;
	int i_desparo;
	Elemento e;
}Nave;

typedef struct {
	TCHAR name;
	int efect;
	Elemento e;
}PowerUP;

typedef struct {
	char username[20];
	int id;
	int powerup;
	int tpowerup;
	Nave nave;
	int points;
	int nvidas;
}Player;

typedef struct {
	int nNavesNormais;
	int nNormaisVivas;
	int nNavesEsquivas;
	int nEsquivasVivas;
	int nPlayers;
	int disparo;
	int dificuldade;
	int maxX; 
	int maxY;
	Nave navesnormais[30];
	Nave navesesquivas[30];
	Player player[5];
}Jogo, *pJogo;

typedef struct {
	int id;
	int efeito;
	Elemento e;
} Powerup;

typedef struct {
	Elemento e;
}Tiro;

typedef struct {
	Elemento e;
}Bomba;

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