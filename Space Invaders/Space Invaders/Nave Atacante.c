#include "utils.h"



DWORD WINAPI thread_basica(LPVOID nave) {
	Nave *a;
	
	a = (Nave *) nave;
	while (a->vida > 0) {

		Sleep(a->velocidade);
		if (a->fimJanela) {
			
			if (can_move()) {
				if (a->fimJanela) {
					a->e.y += a->e.y + a->e.altura;
				}
				else if(a->isLeft)
					a->e.x = a->e.x - 1;
				else a->e.x = a->e.x + 1;
			}
			
		}

	}
	return 0;
}

DWORD WINAPI thread_esquiva(LPVOID nave) {
	Nave *b;
	b = (Nave *)nave;
	
	while (b->vida > 0) {
		Sleep(b->velocidade);

		//random e se sitio esta disponivel



	}

}

