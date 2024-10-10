/* FILE: padreFiglioComunicanti-senzaClose.c: Padre crea un figlio e il figlio si comporta come produttore e il padre come consumatore */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#define MSGSIZE 5	/* lunghezza delle singole linee, stabilita dalla specifica */

int main (int argc, char **argv)
{  	int pid, j, piped[2]; 			/* pid per fork, j per indice, piped per pipe */
	char mess[MSGSIZE];			/* array usato dal figlio per inviare stringa al padre e dal padre per leggere la stringa inviata dal figlio */
						/* NOTA BENE: IL PADRE E IL FIGLIO HANNO LA LORO PROPRIA COPIA DELLA VARIABILE mess NEL PROPRIO SPAZIO DI INDIRIZZAMENTO CHE NON E' ASSOLUTAMENTE CONDIVISO NEI PROCESSI (PESANTI) */
        int pidFiglio, status, ritorno;         /* per wait padre */
        char error[BUFSIZ];     /* usata per scrivere su standard error */

if (argc != 2)
{   	sprintf(error, "Numero dei parametri errato %d: ci vuole un singolo parametro che deve essere il nome di un file\n", argc);
   	perror(error);
    	exit(1);
}

/* IL PARAMETRO PASSATO E' IL NOME DI UN FILE LETTO DAL FIGLIO! */

/* si crea una pipe: ATTENZIONE IL PADRE LA DEVE CREARE PRIMA DI CREARE IL FIGLIO */
if (pipe (piped) < 0 )  
{   	sprintf(error, "Errore creazione pipe\n");
   	perror(error);
    	exit(2); 
}

if ((pid = fork()) < 0)  
{   	sprintf(error, "Errore creazione figlio\n");
   	perror(error);
    	exit(3); 
}
if (pid == 0)  
{   
	/* figlio: LEGGE VIA VIA MSGSIZE CARATTERI DAL FILE E QUESTO SIMULA LA SUA PRODUZIONE DI DATI */
	int fd;
	/* close (piped [0]); 	il figlio NON CHIUDE il lato di lettura: QUINDI NON SI CLASSIFICA COME SCRITTORE DELLA PIPE */
	if ((fd = open(argv[1], O_RDONLY)) < 0)
	{   
		sprintf(error, "Errore in apertura file %s\n", argv[1]);
   		perror(error);
            	exit(-1); /* torniamo al padre un -1 che sara' interpretato come 255 e quindi identificato come errore */
	}

	printf("Figlio %d sta per iniziare a scrivere una serie di messaggi, ognuno di lunghezza %d, sulla pipe dopo averli letti dal file passato come parametro\n", getpid(), MSGSIZE);
	j=0; /* il figlio inizializza la sua variabile j per contare i messaggi che ha mandato al padre: SEMPLICEMENTE PER DARE QUESTA INFORMAZIONE ALL'UTENTE  */
	while (read(fd, mess, MSGSIZE)) /* il contenuto del file e' tale che in mess ci saranno 4 caratteri e il terminatore di linea */
	{   
    		/* il padre ha concordato con il figlio che gli mandera' solo stringhe e quindi dobbiamo sostituire il terminatore di linea con il terminatore di stringa */
    		mess[MSGSIZE-1]='\0'; 
    		write (piped[1], mess, MSGSIZE);
    		j++;
	}
	printf("Figlio %d scritto %d messaggi sulla pipe\n", getpid(), j);
	exit(0);
}

/* padre */
/* close (piped [1]); il padre NON CHIUDE il lato di scrittura: QUINDI NON SI CLASSIFICA COME LETTORE DELLA PIPE */		
printf("Padre %d sta per iniziare a leggere i messaggi dalla pipe\n", getpid());
j=0; /* il padre inizializza la sua variabile j per verificare quanti messaggi ha mandato il figlio */
while (read(piped[0], mess, MSGSIZE))  /* questo ciclo avra' termine appena il figlio terminera' dato che la read senza piu' scrittore tornera' 0! */
{ 	
        /* dato che il figlio gli ha inviato delle stringhe, il padre le puo' scrivere direttamente con una printf */
        printf ("%d: %s\n", j, mess);
        j++;
/* NOTA BENE: delle due close non effettuate (quella nel codice del figlio e quella nel codice del padre)
 e' quella mancante nel codice del padre che produce un blocco del padre, dato che il sistema pensa che ci 
 sia un processo che puo' scrivere sulla pipe e quindi la read da pipe NON torna mai zero e quindi il padre 
 rimane bloccato sulla read in attesa del verificarsi di una condizione che non si puo' verificare dato che 
 l'unico processo che scrive sulla pipe (il figlio) ad un certo punto termina! */
}
printf("Padre %d letto %d messaggi dalla pipe\n", getpid(), j);
/* padre aspetta il figlio */
pidFiglio = wait(&status);
if (pidFiglio < 0)
{
   	sprintf(error, "Errore wait\n");
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
exit(0);
}

