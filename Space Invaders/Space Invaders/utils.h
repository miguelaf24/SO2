#pragma once
#include <time.h>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdbool.h>

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
}PowerUP;

typedef struct {
	Nave navesnormais[10];
	Nave navesesquivas[10];
}Jogo;

typedef struct {
	TCHAR name;
	TCHAR points;
}Player;