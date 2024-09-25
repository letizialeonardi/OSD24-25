#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv)
{       char buffer [BUFSIZ];	/* buffer per leggere dal file o da standard input e scrivere su standard output */
        int nread, fd = 0; 	/* NOTA BENE: fd e' inizializzato a 0 */
	char error[BUFSIZ];	/* usata per scrivere su standard error */

if (argc > 2) 
{	sprintf(error, "Errore nel numero di parametri dato che argc=%d\n", argc);
	perror(error);
        exit(1); 
}

if (argc == 2)
/* abbiamo un parametro che deve essere considerato il nome di un file */
        if ((fd = open(argv[1], O_RDONLY)) < 0)
        {	sprintf(error,"Errore in apertura file %s dato che fd=%d\n", argv[1], fd);
		perror(error);
                exit(2); 
	}

/* se non abbiamo un parametro, allora fd rimane uguale a 0 */
while ((nread = read(fd, buffer, BUFSIZ)) > 0 )
/* lettura dal file o dallo standard input fino a che ci sono caratteri */
	write(1, buffer, nread);
	/* scrittura sullo standard output dei caratteri letti */
exit(0);
}
