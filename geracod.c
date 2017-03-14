/* 	AUTOR :	Lucas Rodrigues 	*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 1024

#define FALSE 0
#define TRUE  1

/****** Prototipo de função que recebe inteiro e retorna inteiro ******/

typedef int (*funcp) (int x);

/****** Prototipos da funcao encapsulada do modulo ******/

static void error (const char * msg, int line);

/****** Prototipos das funcoes do modulo ******/

void geracod (FILE * arq, void ** code, funcp * entry);

void liberacod (void * p);