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
};

typedef struct {
	int tipo;
	bool isLeft;
	bool fimJanela;
	int vida;
	int velocidade;
	int i_desparo;
	struct Elemento e;
}Nave, *pNave;

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
}Player, *pPlayer;

typedef struct {
	pNave navesnormais;
	pNave navesesquivas;
	pPlayer player;
	int dificuldade;
	
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
}Command;

typedef struct {
	int nextIn;	//indice de escrita
	int nextOut; //indice de leitura
	Command buffer[BUFFSIZE];
}Buff, *pBuff;