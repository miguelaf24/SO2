#include "utils.h"

typedef struct {
	bool anda_esperto;
	bool isLeft;
	bool fimJanela;
	int vida;
	int velocidade;
	int i_desparo;
	struct Elemento e;
}Nave;

DWORD WINAPI thread_basica(LPVOID nave) {
	Nave *a;
	
	a = (Nave *) nave;
	while (a->vida > 0) {
		Sleep(a->velocidade);
		if (a->fimJanela) {
			WaitForSingleObject(Mutex, INFINITE);
			if (can_move()) {
				if (a->fimJanela) {
					a->e.y += a->e.y + a->e.altura;
				}
				else if(a->isLeft)
					a->e.x = a->e.x - 1;
				else a->e.x = a->e.x + 1;
			}
			ReleaseMutex(Mutex);
		}
	}
	return 0;
}

DWORD WINAPI thread_esquiva() {
	
}

