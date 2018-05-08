#pragma once
#include <time.h>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdbool.h>

#define BUFFSIZE 30



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
	bool anda_esperto;
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
	Nave navesnormais[10];
	Nave navesesquivas[10];
	Player player1;
	int dificuldade;
}Jogo, *pJogo;

typedef struct {
	struct Elemento e;
}Tiro;

typedef struct {
	struct Elemento e;
}Bomba;

typedef struct {
	struct Elemento e;
	int lifes;
	int velocidade;
	TCHAR name;
	TCHAR points;
}Player;

typedef struct {
	int id;
	int Dir;
}Jogada;

typedef struct Buffer {
	int nextIn;	//indice de escrita
	int nextOut; //indice de leitura
	Jogada buffer[BUFFSIZE];
}Buff, *pBuff;