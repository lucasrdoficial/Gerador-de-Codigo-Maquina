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

/********************** Implementacao das Funcoes ********************/


static void error (const char * msg, int line) 
{
  	fprintf(stderr, "erro %s na linha %d\n", msg, line);
  	exit(EXIT_FAILURE);
}

/* Caso haja falta de memória será enviado somente uma mensagem para o usuário e void ** code = 0 */
void geracod (FILE * arq, void ** code, funcp * entry)
{
	/* Variavel */
	int c, idx_ref, n = 0, func = 0, n_local = 0, line = 1;

	/* Alocacao de memoria para guardar codigo de maquina */
	unsigned char * codigo = (unsigned char *) malloc (sizeof(char) * MAX);
	if (codigo == NULL) /* Verificando se alocacao foi correta */
	{
		printf("Falta Memoria!\n");
		* code = 0;
	}
	/* Vetor para sinalizar uso de variáveis locais */
	unsigned char var_local[5] = {FALSE,FALSE,FALSE,FALSE,FALSE};

	/* Vetor para desvio de call */
	unsigned long call_desvio[50];

	/* Começar a traduzir o código do arquivo */
	while ((c = fgetc(arq)) != EOF)
	{
		switch(c)
		{
			case 'f': /* Function */
			{
				char c0;
				if (fscanf(arq, "unction%c", &c0) != 1)
					error("comando invalido", line);

				call_desvio[func] = (unsigned long) &codigo[n]; /* Guarda endereço de incio de função */
				func ++; /* Atualiza posicao livre do vetor call */

				/* Iniciando a criação do código */
				memcpy(&codigo[n],prologo,sizeof(prologo)); /* Copia prologo para codigo */
				n += sizeof(prologo); /* Atualiza posição livre em codigo */

				memcpy(&codigo[n],nop,sizeof(nop));	/* Copia nop para codigo */
				n += sizeof(nop);	/* Atualiza posição livre em codigo */
				break;
			}
			case 'e': /* End */
			{
				char c0;
				if (fscanf(arq, "nd%c", &c0) != 1)
					error("comando invalido", line);
				break;
			}
			case 'r': /* Retorno */
			{
				int idx0, idx1;
        		char var0, var1;
        		if (fscanf(arq, "et? %c%d %c%d", &var0, &idx0, &var1, &idx1) != 4) 
        		  error("comando invalido", line);
        		if (var0 == '$' && idx0 == 0)
        		{
        			if (var1 == '$')
        			{
        				memcpy(&codigo[n],move_constante,sizeof(move_constante));	/* Copia move_constante para codigo */
			    		n += sizeof(move_constante);		/* Atualiza posição livre em codigo */
						*((int*) &codigo[n]) = idx1;	 /* Insere valor em little endian */
			    		n += 4;		/* Atualiza posição livre em codigo */
        			}
        			else
        			{
        				if (var1 == 'v')
        				{
        					memcpy(&codigo[n],move_variavel,sizeof(move_variavel));		/* Copia move_variavel para codigo */
			    			n += sizeof(move_variavel);		/* Atualiza posição livre em codigo */
			    			codigo[n++]= 0xf8 -8*idx1;	/* Calcula posição da variavel na pilha */
        				}
        				else if (var1 == 'p')
        				{
        					memcpy(&codigo[n],move_parametro,sizeof(move_parametro));		/* Copia move_parametro para codigo */
			    			n += sizeof(move_parametro);	/* Atualiza posição livre em codigo	*/	
			    			codigo[n++]= 0xf8;	/* Guarda edi */
        				}
        			}	
        		}
        		else if (var0 == 'v')
        		{
        			memcpy(&codigo[n],cmp,sizeof(cmp));		/* Copia comparacao para codigo */
		        	n += sizeof(cmp);	/* Atualiza posição livre em codigo */
		        	codigo[n-2] = 0xf8 -8*idx0;		/* Corresponde ao acesso da variavel na pilha */
		        
		        	memcpy(&codigo[n],jne,sizeof(jne));		/* Copia jne para codigo */
		        	n += sizeof(jne);	/* Atualiza posição livre em codigo */

        			idx_ref = n; /* Guarda posiçao para colocar endereço correto no final */
        			n++; /* Atualiza posição livre de codigo para continuar */

        			if (var1 == '$')
        			{
        				memcpy(&codigo[n],move_constante,sizeof(move_constante));	/* Copia move_constante para codigo */
			    		n += sizeof(move_constante);		/* Atualiza posição livre em codigo */
						*((int*) &codigo[n]) = idx1;	 /* Insere valor em little endian */
			    		n += 4;		/* Atualiza posição livre em codigo */
        			}
        			else
        			{
        				if (var1 == 'v')
        				{
        					memcpy(&codigo[n],move_variavel,sizeof(move_variavel));		/* Copia move_variavel para codigo */
			    			n += sizeof(move_variavel);		/* Atualiza posição livre em codigo */
			    			codigo[n++]= 0xf8 -8*idx1;	/* Calcula posição da variavel na pilha */
        				}
        				else if (var1 == 'p')
        				{
        					memcpy(&codigo[n],move_parametro,sizeof(move_parametro));		/* Copia move_parametro para codigo */
			    			n += sizeof(move_parametro);	/* Atualiza posição livre em codigo	*/	
			    			codigo[n++]= 0xf8;	/* Considera edi */
        				}
        			}
        			codigo[idx_ref] = (unsigned char)(&codigo[n] - (&codigo[idx_ref] + 1)); /* Calculo para jump to ret */
        		}
        		else if (var0 == 'p')
        		{
        			memcpy(&codigo[n],cmp_parametro,sizeof(cmp_parametro));		/* Copia comparacao para codigo */
		        	n += sizeof(cmp_parametro);	/* Atualiza posição livre em codigo */
		        	codigo[n-2] = 0xf8;		/* Considera edi */
		        
		        	memcpy(&codigo[n],jne,sizeof(jne));		/* Copia jne para codigo */
		        	n += sizeof(jne);	/* Atualiza posição livre em codigo */

        			idx_ref = n; /* Guarda posiçao para colocar endereço correto no final */
        			n ++; /* Atualiza posição livre de codigo para continuar */

        			if (var1 == '$')
        			{
        				memcpy(&codigo[n],move_constante,sizeof(move_constante));	/* Copia move_constante para codigo */
			    		n += sizeof(move_constante);		/* Atualiza posição livre em codigo */
						*((int*) &codigo[n]) = idx1;	 /* Insere valor em little endian */
			    		n += 4;		/* Atualiza posição livre em codigo */
        			}
        			else
        			{
        				if (var1 == 'v')
        				{
        					memcpy(&codigo[n],move_variavel,sizeof(move_variavel));		/* Copia move_variavel para codigo */
			    			n += sizeof(move_variavel);		/* Atualiza posição livre em codigo */
			    			codigo[n++]= 0xf8 -8*idx1;	/* Calcula posição da variavel na pilha */
        				}
        				else if (var1 == 'p')
        				{
        					memcpy(&codigo[n],move_parametro,sizeof(move_parametro));		/* Copia move_parametro para codigo */
			    			n += sizeof(move_parametro);	/* Atualiza posição livre em codigo	*/	
			    			codigo[n++]= 0xf8;	/* Considera edi */
        				}
        			}
        			codigo[idx_ref] = (unsigned char)(&codigo[n] - (&codigo[idx_ref] + 1)); /* Calculo para jump to ret */
        		}
        		memcpy(&codigo[n],ret,sizeof(ret));	 /* Copia leave - ret para codigo */
			    n += sizeof(ret);	/* Atualiza posição livre em codigo */
        		break;
			}
			case 'v': /* Atribuição */
			{
        		int idx0;
        		char c0;
        		if (fscanf(arq, "%d = %c",&idx0, &c0) != 2)
        		  error("comando invalido",line);
        		if (var_local[idx0] == FALSE) /* Verifica se variável foi contabilizada */
        		{
        			var_local[idx0] = TRUE;
        			n_local++;  /* Contabiliza Variável */
        		}

        		if (c0 == 'c') /* Call */
        		{
          			int f, idx1;
          			char var1;
          			if (fscanf(arq, "all %d %c%d\n", &f, &var1, &idx1) != 3)
            			error("comando invalido",line);
	            		if (var1 == '$')
	            		{
	            			memcpy(&codigo[n],constante2par,sizeof(constante2par));	/* Copia constante2par para codigo */
         	 			n += sizeof(constante2par);		/* Atualiza posição livre em codigo */
         	 			*((int *) &codigo[n]) = idx1;	/* Insere valor em little endian */
			    		n += 4;		/* Atualiza posição livre em codigo */
	            		}
	            		else if (var1 == 'v')
	            		{
	            			memcpy(&codigo[n],variavel2par,sizeof(variavel2par));		/* Copia variavel2par para codigo */
         	 			n += sizeof(variavel2par);		/* Atualiza posição livre em codigo */
         	 			codigo[n++]= 0xf8 -8*idx1;	/* Calcula posição da variavel na pilha */
	            		}
        			memcpy(&codigo[n],call,sizeof(call));	 /* Copia call para codigo */
			    	n += sizeof(call);	/* Atualiza posição livre em codigo */
			    	*((int *) &codigo[n]) = (unsigned long)call_desvio[f] - (unsigned long)&codigo[n+4]; /* Calculo para jump to ret */
			    	n++; /* Atualiza posição livre em codigo */
	        	}