#ifndef	 GERA_H
#define GERA_H

typedef int (*funcp) (int x);
void geracod (FILE *f, void **code, funcp *entry);
void liberacod (void *p);

#endif
