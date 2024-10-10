#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

#define PERM 0644

int main (int argc, char **argv)
{
        int N;                          /* numero di figli */
        int pid;                        /* pid per fork */
        int i;                          /* indice */
        char *FOut;                     /* nome del file da creare da parte dei figli */
        int fdw;                        /* per la creat */
        int pidFiglio, status, ritorno; /* per valore di ritorno figli */
        char error[BUFSIZ];     /* usata per scrivere su standard error */


/* controllo sul numero di parametri: almeno tre nomi file-N- */
if (argc < 4)
{
        sprintf(error, "Errore numero di parametri: i parametri passati a %s sono solo %d\n", argv[0], argc);
	perror(error);
        exit(1);
}

/* individuiamo il numero di file/processi */
N=argc-1;

printf("Sono il processo padre con pid %d e creero' %d processi figli che generanno ognuno un nipote\n", getpid(), N);

/* creazione figli */
for (i=0;i<N;i++)
{
        if ((pid=fork())<0)
        {
                sprintf(error, "Errore creazione figlio %d-esimo\n", i);
		perror(error);
                exit(3);
        }
        else if (pid==0)
        {       /* codice figlio */
                printf("Sono il figlio %d di indice %d\n", getpid(), i);
                /* in caso di errore (sia nei figli che nei nipoti) decidiamo di ritornare -1 che sara' interpretato dal padre come 255 e quindi un valore non valido! */
                /* i figli devono creare il file specificato */
                FOut=(char *)malloc(strlen(argv[i+1]) + 6); /* bisogna allocare una stringa lunga come il nome del file + il carattere '.' + i caratteri della parola sort (4) + il terminatore di stringa */
                if (FOut == NULL)
                {
                        sprintf(error, "Errore nelle malloc\n");
			perror(error);
                        exit(-1);
                }
                /* copiamo il nome del file associato al figlio */
                strcpy(FOut, argv[i+1]);
                /* concateniamo la stringa specificata dal testo */
                strcat(FOut,".sort");
                fdw=creat(FOut, PERM);
                if (fdw < 0)
                {
                        sprintf(error, "Impossibile creare il file %s\n", FOut);
			perror(error);
                        exit(-1);
                }
                /* chiudiamo il file creato che il figlio non usa */
                close(fdw);

                if ( (pid = fork()) < 0) /* ogni figlio crea un nipote */
                {
                        sprintf(error, "Errore nella fork di creazione del nipote\n");
			perror(error);
                        exit(-1);
                }
                if (pid == 0)
                {
                        /* codice del nipote */
                        printf("Sono il processo nipote del figlio di indice %d e pid %d sto per eseguire il comando sort per il file %s\n", i, getpid(), argv[i+1]);
			/* la specifica dice che si deve usare il FILTRO sort e quindi bisogna operare sia la ridirezione dello standard input che dello standard output */
                        /* chiudiamo lo standard input */
                        close(0);
                        /* apriamo il file associato in sola lettura */
                        if (open(argv[i+1], O_RDONLY) < 0)
                        {
                                sprintf(error, "Errore: FILE %s NON ESISTE\n", argv[i+1]);
				perror(error);
                                exit(-1);
                        }
                        /* chiudiamo lo standard output */
                        close(1);
                        /* apriamo il file creato in sola scrittura */
                        if (open(FOut, O_WRONLY) < 0)
                        {
                                sprintf(error, "Errore: FILE %s NON si riesce ad aprire in scrittura\n", FOut);
				perror(error);
                                exit(-1);
                        }
                        /* Il nipote diventa il comando sort: bisogna usare le versioni dell'exec con la p in fondo in modo da usare la variabile di ambiente PATH: NON serve alcun parametro */
                        execlp("sort", "sort", (char *)0);

                        /* Non si dovrebbe mai tornare qui!!*/
                       perror("Problemi di esecuzione del sort da parte del nipote");
                       exit(-1);
                }
                /* il figlio deve aspettare il nipote per ritornare al padre il valore tornato dal nipote  */
                pid = wait(&status);
                if (pid < 0)
                {
                        sprintf(error, "Errore in wait\n");
			perror(error);
                        exit(-1);
                }
                if ((status & 0xFF) != 0)
                {
                        sprintf(error, "Nipote con pid %d terminato in modo anomalo\n", pid);
			perror(error);
                        exit(-1);
                }
                else
                        ritorno=(int)((status >> 8) & 0xFF);
                /* il figlio ritorna il valore ricevuto dal nipote  */
                exit(ritorno);
                }
} /* fine for */

/* codice del padre */
/* Il padre aspetta i figli */
for (i=0; i < N; i++)
{
        pidFiglio = wait(&status);
        if (pidFiglio < 0)
        {
                sprintf(error, "Errore in wait\n");
		perror(error);
                exit(4);
        }
        if ((status & 0xFF) != 0)
                printf("Figlio con pid %d terminato in modo anomalo\n", pidFiglio);
        else
        {
                ritorno=(int)((status >> 8) & 0xFF);
                printf("Il figlio con pid=%d ha ritornato %d (se 255 problemi!)\n", pidFiglio, ritorno);
        }
}

exit(0);
}

