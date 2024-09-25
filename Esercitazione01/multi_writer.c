#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

typedef enum { FALSE, TRUE } Boolean;

int main(int argc, char *argv[])
{
    int pid, status, ritorno;			/* per fork e wait */
    char *buf;					/* buffer di byte che ogni figlio scrive */
    int j, k; 					/* indici */
    int nblocks, nchildren;			/* nblocks numero di blocchi da scrivere; nchildren numero di figli da creare */
    int blocksize;				/* dimensione di ogni blocco di byte */
    long long expected;				/* lunghezza calcolata sulla base del numero di blocchi e della loro lunghzza */
    long long calculated;			/* lunghezza calcolata con lseek */
    char error[200];				/* stringa per stampe fatte su standard error perche' lo standard output verra' rediretto */

    if (argc != 4)
    {
        sprintf(error, "Numero sbagliato parametri (necessari 3 parametri, numero figli, numero blocchi, grandezza blocchi %s\n", argv[0]);
        perror(error);
	exit(1);
    }  

    nblocks = atoi(argv[2]);
    blocksize = atoi(argv[3]);
    nchildren = atoi(argv[1]);
    
    if (nchildren <= 0 || nblocks <= 0 || blocksize <= 0) 
    {
	perror("un qualche numero non e' strettamente positivo");
	exit(2);
    }


    buf = malloc(blocksize);
    if (buf == NULL)
    {
        perror("malloc");
	exit(3);
    }  


    /* Creaiamo i figli che scrivono i blocchi sullo standard output (RICORDARSI DI USARE LA RIDIREZIONE IN OUTPUT */
    for (j = 0; j < nchildren; j++) 
    {
        if ((pid=fork()) < 0)
        {
        	perror("fork");
		exit(4);
        }

        if (pid == 0) /* Ogni figlio scrive nblocks * blocksize byte sullo standard output (cioe' 1): notare che l'I/O pointer e' condiviso fra padre e figli e fra tutti i figli fra di loro */
        {
            /* Mettiamo qualcosa di distintivo in ogni buffer (nel caso vogliamo analizzare la sequenza di byte nello standard output) */

            for (k = 0; k < blocksize; k++)
                buf[k] = 'a' + getpid() % 26;

            for (k = 0; k < nblocks; k++) 
	    {
                if (write(1, buf, blocksize) != blocksize) /* ogni singola write e' atomica e avanza l'I/O pointer per tutti i processi (padre e figli) */
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
    for (j = 0; j < nchildren; j++) 
    {
     	pid = wait(&status);
        if (pid < 0)
        {
        	perror("Errore in wait\n");
                exit(5);
        }

        if ((status & 0xFF) != 0)
    	{
                sprintf(error, "Figlio con pid %d terminato in modo anomalo\n", pid);
	    	perror(error);
        }
        else
        { 
		ritorno=(int)((status >> 8) & 0xFF);
                sprintf(error,"Il figlio con pid=%d ha ritornato %d (se 255 problemi nel figlio)\n", pid, ritorno);
                perror(error);
        }
    }

    /* Controlliamo la lunghezza finale del file rispetto alla lunghezza che dovrebbe essere */
    expected =  blocksize * nblocks * nchildren;
    sprintf(error, "Lunghezza che dovrebbe avere il file: %10lld\n", expected);
    perror(error);
    calculated=lseek(1, 0L, SEEK_END);
    sprintf(error, "Lunghezza attuale file: %10lld\n", (long long) calculated);
    perror(error);
    sprintf(error, "Differenza: %10lld\n", expected - calculated);
    perror(error);

    exit(0);
}

