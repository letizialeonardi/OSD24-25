#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

int main  (int argc, char **argv)
{       int i, n;       	/* i serve per contare le linee, n per sapere quante linee devono essere mostrate (deriva dall'opzione) */
        char c;         	/* per leggere i caratteri da standard input o da file e scriverli su standard output */
        int fd;         	/* per la open */
        int par = 0;    	/* per tenere traccia se e' usato un nome di file o meno  */
        char *op, *nf;  	/* per rendere piu' leggibile il codice */
	char error[BUFSIZ];     /* usata per scrivere su standard error */


/* controllo numero di parametri */
if (argc > 3)
{
	sprintf(error, "Errore: Necessario 0, 1 oppure 2 argomenti per %s, dato che argc=%d\n", argv[0], argc);
	perror(error);
        exit(1);
}
switch (argc)
{
        case 3: op = argv[1]; nf = argv[2];
                par = 1;
                if (op[0] != '-')
                {
                        sprintf(error, "Errore: Necessario il simbolo di opzione\n");
			perror(error);
                        exit(2);
                }
                else
                {
                        n = atoi(&(op[1]));
                        if (n <= 0)
                        {
                                sprintf(error, "Errore: l'opzione non e' corretta\n");
				perror(error);
                                exit(3);
                        }
                }
                break;

        case 2: op = argv[1];
                if (op[0] != '-') { nf = op; n = 10; par = 1;}
                else
                {
                        n = atoi(&(op[1]));
                        if (n <= 0)
                        {
                                sprintf(error, "Errore: l'opzione non e' correttai dato che n=%d\n", n);
				perror(error);
                                exit(4);
                        }
                }
                break;

        case 1: n = 10;  break;
}

if (par == 1)
{       fd = open(nf, O_RDONLY);
        if (fd < 0)
        {
         	sprintf(error,"Errore in apertura file %s dato che fd=%d\n", nf, fd);
		perror(error);
		exit(5);
	}
}
else
        fd = 0; /* si deve considerare lo standard input nella lettura */

i = 1; /* inizializzo il conteggio delle linee a 1 */
while (read (fd, &c, 1) != 0)
      {         if (c == '\n') i++;     /* se troviamo un terminatore di linea incrementiamo il conteggio */
                write(1, &c, 1);        /* scriviamo comunque il carattere qualunque sia */
                if (i > n) break;       /* se il conteggio supera n allora usciamo dal ciclo di lettura */
      }
exit(0);
}
