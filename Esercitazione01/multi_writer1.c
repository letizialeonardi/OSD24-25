#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#define PERM 0644   /* in UNIX */

typedef enum { FALSE, TRUE } Boolean;

int main(int argc, char *argv[])
{
    int pid, status, ritorno;			/* per fork e wait */
    char *buf;					/* buffer di byte che ogni figlio scrive */
    int j, k; 					/* indici */
    int nblocks, nchildren;			/* nblocks numero di blocchi da scrivere; nchildren numero di figli da creare */
    int blocksize;				/* dimensione di ogni blocco di byte */
    int fd;     				/* file descriptor per file creato dal padre */
    long long expected;				/* lunghezza calcolata sulla base del numero di blocchi e della loro lunghzza */
    long long calculated;			/* lunghezza calcolata con lseek */
    char error[200];				/* stringa per stampe fatte su standard error */

    if (argc != 5)
    {
        sprintf(error, "Numero sbagliato parametri (necessari 4 parametri, numero figli, numero blocchi, grandezza blocchi, nome file %s\n", argv[0]);
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

    /* creiamo il file il cui nome è passato come quarto parametro */ 
    if ((fd = creat (argv[4], PERM)) < 0 )
	/* ERRORE se non si riesce a creare il file */
    {
        perror("creat");
        exit(4);
    }

    /* Creiamo i figli che scrivono i blocchi sul file */
    for (j = 0; j < nchildren; j++) 
    {
        if ((pid=fork()) < 0)
        {
        	perror("fork");
		exit(5);
        }

        if (pid == 0) /* Ogni figlio scrive nblocks * blocksize byte sul file creato dal padre: notare che l'I/O pointer e' condiviso fra padre e figli e fra tutti i figli fra di loro */
        {
            /* Mettiamo qualcosa di distintivo in ogni buffer (nel caso vogliamo analizzare la sequenza di byte nel file) */

            for (k = 0; k < blocksize; k++)
                buf[k] = 'a' + getpid() % 26;

            for (k = 0; k < nblocks; k++) 
	    {
                if (write(fd, buf, blocksize) != blocksize) /* ogni singola write e' atomica e avanza l'I/O pointer per tutti i processi (padre e figli) */
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

    /* Controlliamo la lunghezza finale del file rispetto alla lunghezza che dovrebbe essere */
    /* NOTA BENE: in questa versione per queste scritture su standard output si puo' usare la normale printf */
    expected =  blocksize * nblocks * nchildren;
    printf("Lunghezza che dovrebbe avere il file: %10lld\n", expected);
    calculated=lseek(fd, 0L, SEEK_END);
    printf("Lunghezza attuale file: %10lld\n", (long long) calculated);
    printf("Differenza: %10lld\n", expected - calculated);

    exit(0);
}

