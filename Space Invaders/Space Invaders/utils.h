#pragma once
#include <time.h>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdbool.h>

#define BUFFSIZE 30

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
	TCHAR id[3];
};

typedef struct {
	bool isLeft;
	bool fimJanela;
	int vida;
	int velocidade;
	int i_desparo;
	struct Elemento e;
}Nave;

typedef struct {
	TCHAR name;
	int efect;
	struct Elemento e;
}PowerUP;

typedef struct {
	TCHAR name[TCHARSIZE];
	int powerup;
	int tpowerup;
	Nave nave;
	int points;
	int nvidas;
}Player;

typedef struct {
	int nNavesNormais;
	int nNormaisVivas;
	Nave navesnormais[30];
	int nNavesEsquivas;
	int nEsquivasVivas;
	Nave navesesquivas[30];
	int nPlayers;
	Player player[5];
	int disparo;
	int dificuldade;
	int maxX; 
	int maxY;
}Jogo, *pJogo;

typedef struct {
	int id;
	int efeito;
	Elemento e;
} Powerup, *pPowerup;

typedef struct {
	struct Elemento e;
}Tiro;

typedef struct {
	struct Elemento e;
}Bomba;

typedef struct {
	int id;
	int cmd;
}Command, *pCommand;

typedef struct {
	int nextIn;	//indice de escrita
	int nextOut; //indice de leitura
	Command buffer[BUFFSIZE];
}Buff, *pBuff;