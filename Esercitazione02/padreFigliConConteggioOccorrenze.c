#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

int main (int argc, char **argv)
{
        char Cx;                        /* carattere che i figli devono cercare nel file a loro associato */
        int N;                          /* numero di figli */
        int pid;                        /* pid per fork */
        int i;                          /* indice */
        int totale=0;                   /* serve per calcolare il numero di occorrenze: in questo caso abbiamo usato un semplice int perche' la specifica dice che si puo' supporre minore di 255 */
        int fd;                         /* per la open */
        char c;                         /* per leggere i caratteri dal file */
        int pidFiglio, status, ritorno; /* per valore di ritorno figli */
        char error[BUFSIZ];     	/* usata per scrivere su standard error */


/* controllo sul numero di parametri: almeno due nomi file-N- e un carattere */
if (argc < 4)
{
        sprintf(error, "Errore numero di parametri: i parametri passati a %s sono solo %d\n", argv[0], argc);
	perror(error);
        exit(1);
}

/* controlliamo che l'ultimo parametro sia un singolo carattere */
if (strlen( argv[argc-1]) != 1)
{
        sprintf(error, "Errore ultimo parametro non singolo carattere dato che e' %s\n", argv[argc-1]);
	perror(error);
        exit(2);
}

/* individuiamo il carattere da cercare */
Cx = argv[argc-1][0];

/* individuiamo il numero di file/processi */
N=argc-2;

printf("Sono il processo padre con pid %d e creero' %d processi figli che cercheranno il carattere %c nei file passati come parametri\n", getpid(), N, Cx);

/* creazione figli */
for (i=0;i<N;i++)
{
        if ((pid=fork())<0)
        {
                sprintf(error, "Errore creazione figlio\n");
		perror(error);
                exit(3);
        }
        else if (pid==0)
        {       /* codice figlio */
                printf("Sono il figlio %d di indice %d associato al file %s\n", getpid(), i, argv[i+1]);
                /* apriamo il file (deleghiamo ad ogni processo figlio, il controllo che i singoli parametri (a parte l'ultimo) siano nomi di file */
                /* notare che l'indice che dobbiamo usare e' i+1 */
                /* in caso di errore decidiamo di ritornare -1 che sara' interpretato dal padre come 255 e quindi un valore non valido! */
                if ((fd = open(argv[i+1], O_RDONLY)) < 0)
                {
                        sprintf(error, "Errore: FILE %s NON ESISTE\n", argv[i+1]);
			perror(error);
                        exit(-1);
                }
                /* leggiamo il file */
                while (read (fd, &c, 1) != 0)
                         if (c == Cx) totale++;     /* se troviamo il carattere increntiamo il conteggio */

                 /* ogni figlio deve tornare il numero di occorrenze e quindi totale */
                exit(totale);
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

