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

/******* Array de codigos ********/

static unsigned char nop           [] = {0x90,0x90,0x90,0x90}; /* área reservada para possível alocação de espaço na pilha para variáveis locais */
static unsigned char prologo       [] = {0x55,0x48,0x89,0xe5}; /* pushq %rbp - movq %rsp, %rbp */
static unsigned char alocacao      [] = {0x48,0x83,0xec}     ; /* subq $constante, %rsp */
static unsigned char cmp           [] = {0x83,0x7d,0x00,0x00}; /* cmpl $0, var */
static unsigned char cmp_parametro [] = {0x83,0xff,0x00,0x00}; /* cmpl $0, %rdi */
static unsigned char atribuicao    [] = {0x89,0x45}          ; /* movl %eax, var */
static unsigned char constante2par [] = {0xbf}               ; /* movl $constante, %edi */
static unsigned char variavel2par  [] = {0x8b,0x7d}          ; /* movl var, %edi */
static unsigned char move_constante[] = {0xb8}               ; /* movl $constante, %eax */
static unsigned char move_parametro[] = {0x89}               ; /* movl %parametro (%edi), %eax */
static unsigned char move_variavel [] = {0x8b,0x45}          ; /* movl var, %eax */
static unsigned char add_constante [] = {0x05}               ; /* addl $constante, %eax */
static unsigned char add_parametro [] = {0x01}               ; /* addl %parametro (%edi), %eax */
static unsigned char add_variavel  [] = {0x03,0x45}          ; /* addl var, %eax */
static unsigned char sub_constante [] = {0x2d}               ; /* subl $constante, %eax */
static unsigned char sub_variavel  [] = {0x2b,0x45}          ; /* subl var, %eax */
static unsigned char sub_parametro [] = {0x29}               ; /* subl %parametro (%edi), %eax */
static unsigned char mult_constante[] = {0x69,0xc0}          ; /* imull $constante, %eax */
static unsigned char mult_variavel [] = {0x0f,0xaf,0x45}     ; /* imull var, %eax */
static unsigned char mult_parametro[] = {0x0f,0xaf}          ; /* imull %parametro(%edi), %eax */
static unsigned char call          [] = {0xe8}               ; /* call funcao */
static unsigned char jne           [] = {0x75}               ; /* desvio condicional -> if (var != 0) */
static unsigned char ret           [] = {0xc9,0xc3}          ; /* leave - ret */