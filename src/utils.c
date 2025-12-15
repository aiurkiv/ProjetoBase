#include "utils.h"

/*
	isqrt32()

	Funçãop pra calcular um raiz inteira de um número inteiro.
	Baseada em:
	www.quora.com/Which-is-the-fastest-algorithm-to-compute-integer-square-root-of-a-number
	www.codecodex.com/wiki/Calculate_an_integer_square_root
 */
uint32_t isqrt32(uint32_t n)
{
	uint32_t root, remainder, place;

	root = 0;
	remainder = n;
	place = 0x400000;

	while (place > remainder)
		place = place >> 2;
    
	while (place)
    {
		if (remainder >= root + place)
        {
			remainder = remainder - root - place;
			root = root + (place << 1);
		}
		root = root >> 1;
		place = place >> 2;
	}
	return root;
}

uint32_t calcula_rms(uint32_t valor)
{
    uint32_t soma=0;

	soma = valor;	
	// Divide por 128
	soma = soma >> 7;
	// Se for 0 nem perde tempo
	if(soma)
		soma = isqrt32(soma);
	return soma;
}