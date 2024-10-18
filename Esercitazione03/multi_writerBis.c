#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <string.h>
#define PERM 0644   /* in UNIX */

int main(int argc, char *argv[])
{
    int pid, status, ritorno;			/* per fork e wait */
    char ch;					/* carattere usato da ogni figlio per scrivere */
    int j, k; 					/* indici */
    int nblocks;				/* nblocks numero di caratteri da scrivere */
    int fd;     				/* file descriptor per file creato dal padre */
    char error[200];				/* stringa per stampe fatte su standard error */

    if (argc != 3)
    {
        sprintf(error, "Numero sbagliato parametri (necessari 2 parametri, numero blocchi e nome file %s\n", argv[0]);
        perror(error);
	exit(1);
    }  

    nblocks = atoi(argv[1]);

    if (nblocks <= 0)
    {
        perror("nblocks non e' strettamente positivo");
        exit(2);
    }

    /* creiamo il file il cui nome è passato come secondo parametro */ 
    if ((fd = creat(argv[2], PERM)) < 0 )
	/* ERRORE se non si riesce a creare il file */
    {
        perror("creat");
        exit(4);
    }
    close(fd); /* il padre chiude il file prima della creazione dei figli dato che questa soluzione NON utilizza la condivisione del I/O pointer */

    /* Creiamo i figli che scrivono i caratteri sul file */
    for (j = 0; j < 10; j++) 
    {
        if ((pid=fork()) < 0)
        {
        	perror("fork");
		exit(5);
        }

        if (pid == 0) /* Ogni figlio scrive nblocks byte sul file creato dal padre: notare che l'I/O pointer NON e' condiviso dato che ogni figlio riapre il file e sposta l'I/O pointer alla fine */
        {
    		/* apriamo il file in scrittura */ 
    		if ((fd = open(argv[2], O_WRONLY)) < 0 )
		/* ERRORE se non si riesce a creare il file */
    		{
        		perror("open");
			exit(-1); /* torniamo -1 che sara' interpretato come errore (dato che se va tutto bene i figli tornano 0) */
        	}

	    /* Ci spostiamo alla fine del file */	
    	    lseek(fd, 0L, SEEK_END);

            for (k = 0; k < nblocks; k++) 
	    {
             	/* Scriviamo qualcosa di distintivo nel file: in questo caso il carattere numerico corrispondente all'indice del processo */
		ch='0'+j;
                if (write(fd, &ch, 1) != 1) /* ogni singola write e' atomica e avanza l'I/O pointer ma solo per questo processo */
    		{
        		perror("write");
			exit(-1); /* torniamo -1 che sara' interpretato come errore (dato che se va tutto bene i figli tornano 0) */
    		}  
            
	    }

            exit(0);
        }
    }

    /* CODICE DEL PADRE */
    
   /* Aspetta la terminazione di tutti i figli  */
    for (j = 0; j < 10; j++) 
    {
     	pid = wait(&status);
        if (pid < 0)
        {
        	perror("Errore in wait\n");
                exit(6);
        }

        if ((status & 0xFF) != 0)
    	{
                sprintf(error, "Figlio con pid %d terminato in modo anomalo\n", pid);
	    	perror(error);
        }
        else
        { 
		ritorno=(int)((status >> 8) & 0xFF);
	    	/* NOTA BENE: in questa versione per queste scritture su standard output si puo' usare la normale printf */
                printf("Il figlio con pid=%d ha ritornato %d (se 255 problemi nel figlio)\n", pid, ritorno);
        }
    }

    exit(0);
}

